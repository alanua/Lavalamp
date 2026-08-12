#!/usr/bin/env python3
"""Openbox/X11 idle controller for the local Lavalamp generative renderer.

The controller never changes lock/authentication or DPMS policy. It reads the X11
ScreenSaver extension for real input idle time, reads logind LockedHint for session
lock state, and starts the visual renderer only when the canonical media ownership
guard explicitly reports CLEAR.
"""
from __future__ import annotations

import argparse
import ctypes
import ctypes.util
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import time

LOGINCTL = "/usr/bin/loginctl"
DEFAULT_ROOT = Path.home() / ".local/lib/lavalamp-home-edge"
DEFAULT_GUARD = Path.home() / ".local/bin/home-edge-media-display-owner"
DEFAULT_LAUNCHER = DEFAULT_ROOT / "home_edge/screensaver/launch-renderer.sh"


def _positive_float(name: str, default: float, minimum: float, maximum: float) -> float:
    raw = os.environ.get(name, "").strip()
    try:
        value = float(raw) if raw else default
    except ValueError:
        value = default
    return max(minimum, min(maximum, value))


IDLE_SECONDS = _positive_float("SKELETON_SCREENSAVER_IDLE_SECONDS", 120.0, 10.0, 3600.0)
POLL_SECONDS = _positive_float("SKELETON_SCREENSAVER_POLL_SECONDS", 1.0, 0.25, 10.0)
POST_MEDIA_GRACE_SECONDS = _positive_float(
    "SKELETON_SCREENSAVER_POST_MEDIA_GRACE_SECONDS", IDLE_SECONDS, 10.0, 3600.0
)
RESTART_BACKOFF_SECONDS = _positive_float(
    "SKELETON_SCREENSAVER_RESTART_BACKOFF_SECONDS", 10.0, 2.0, 300.0
)

# Canonical Debian media bootstrap is LightDM + Openbox/Xorg. systemd --user may
# start before the graphical environment has imported these values, so use only
# fixed local defaults and fail closed until the X server/auth cookie is readable.
os.environ.setdefault("DISPLAY", ":0")
os.environ.setdefault("XAUTHORITY", str(Path.home() / ".Xauthority"))


def _run(argv: list[str], timeout: float = 3.0) -> subprocess.CompletedProcess[str]:
    return subprocess.run(argv, text=True, capture_output=True, timeout=timeout, check=False)


class _XScreenSaverInfo(ctypes.Structure):
    _fields_ = [
        ("window", ctypes.c_ulong),
        ("state", ctypes.c_int),
        ("kind", ctypes.c_int),
        ("since", ctypes.c_ulong),
        ("idle", ctypes.c_ulong),
        ("event_mask", ctypes.c_ulong),
    ]


def x11_idle_ms() -> int:
    x11_name = ctypes.util.find_library("X11") or "libX11.so.6"
    xss_name = ctypes.util.find_library("Xss") or "libXss.so.1"
    try:
        x11 = ctypes.CDLL(x11_name)
        xss = ctypes.CDLL(xss_name)
    except OSError as exc:
        raise RuntimeError("x11_idle_library_unavailable") from exc

    x11.XOpenDisplay.argtypes = [ctypes.c_char_p]
    x11.XOpenDisplay.restype = ctypes.c_void_p
    x11.XDefaultRootWindow.argtypes = [ctypes.c_void_p]
    x11.XDefaultRootWindow.restype = ctypes.c_ulong
    x11.XCloseDisplay.argtypes = [ctypes.c_void_p]
    x11.XCloseDisplay.restype = ctypes.c_int
    x11.XFree.argtypes = [ctypes.c_void_p]
    x11.XFree.restype = ctypes.c_int
    xss.XScreenSaverAllocInfo.argtypes = []
    xss.XScreenSaverAllocInfo.restype = ctypes.POINTER(_XScreenSaverInfo)
    xss.XScreenSaverQueryInfo.argtypes = [
        ctypes.c_void_p,
        ctypes.c_ulong,
        ctypes.POINTER(_XScreenSaverInfo),
    ]
    xss.XScreenSaverQueryInfo.restype = ctypes.c_int

    display_name = os.environ.get("DISPLAY", ":0").encode("utf-8")
    display = x11.XOpenDisplay(display_name)
    if not display:
        raise RuntimeError("x11_display_unavailable")
    info = xss.XScreenSaverAllocInfo()
    if not info:
        x11.XCloseDisplay(display)
        raise RuntimeError("x11_idle_alloc_failed")
    try:
        root = x11.XDefaultRootWindow(display)
        if not xss.XScreenSaverQueryInfo(display, root, info):
            raise RuntimeError("x11_idle_query_failed")
        return int(info.contents.idle)
    finally:
        x11.XFree(ctypes.cast(info, ctypes.c_void_p))
        x11.XCloseDisplay(display)


def _session_property(session_id: str, prop: str) -> str:
    process = _run([LOGINCTL, "show-session", session_id, f"--property={prop}", "--value"], timeout=2.0)
    if process.returncode != 0:
        raise RuntimeError("logind_session_query_failed")
    return process.stdout.strip().lower()


def _active_session_id() -> str:
    explicit = os.environ.get("XDG_SESSION_ID", "").strip()
    if explicit:
        try:
            if _session_property(explicit, "Active") == "yes":
                return explicit
        except Exception:
            pass
    process = _run([LOGINCTL, "list-sessions", "--no-legend", "--no-pager"], timeout=2.0)
    if process.returncode != 0:
        raise RuntimeError("logind_session_list_failed")
    uid = str(os.getuid())
    candidates: list[str] = []
    for line in process.stdout.splitlines():
        fields = line.split()
        if len(fields) >= 2 and fields[1] == uid and re.fullmatch(r"[A-Za-z0-9_.-]+", fields[0]):
            candidates.append(fields[0])
    for session_id in candidates:
        try:
            if _session_property(session_id, "Active") == "yes":
                return session_id
        except Exception:
            continue
    raise RuntimeError("active_logind_session_unavailable")


def session_locked() -> bool:
    value = _session_property(_active_session_id(), "LockedHint")
    if value == "yes":
        return True
    if value == "no":
        return False
    raise RuntimeError("logind_lock_state_unknown")


def media_ownership() -> str:
    """Return OWNER, CLEAR or UNKNOWN from the canonical Home Edge guard.

    Exit 0 means a confirmed video session owns the display; exit 1 means explicitly
    clear; every other result fails closed. No title/URL/account/session payload is
    consumed by this controller.
    """
    guard = Path(os.environ.get("SKELETON_SCREENSAVER_MEDIA_GUARD", str(DEFAULT_GUARD))).expanduser()
    if not guard.is_file() or not os.access(guard, os.X_OK):
        return "UNKNOWN"
    try:
        process = _run([str(guard), "status"], timeout=2.0)
    except (OSError, subprocess.SubprocessError):
        return "UNKNOWN"
    if process.returncode == 0:
        return "OWNER"
    if process.returncode == 1:
        return "CLEAR"
    return "UNKNOWN"


def _launcher_path() -> Path:
    return Path(os.environ.get("SKELETON_SCREENSAVER_LAUNCHER", str(DEFAULT_LAUNCHER))).expanduser()


def _public_log(event: str, **fields: object) -> None:
    payload = {"component": "skeleton-generative-saver", "event": event, **fields}
    print(json.dumps(payload, sort_keys=True, separators=(",", ":")), flush=True)


class RendererProcess:
    def __init__(self) -> None:
        self.process: subprocess.Popen[str] | None = None

    @property
    def running(self) -> bool:
        return self.process is not None and self.process.poll() is None

    def start(self) -> bool:
        if self.running:
            return False
        launcher = _launcher_path()
        if not launcher.is_file() or not os.access(launcher, os.X_OK):
            raise RuntimeError("launcher_unavailable")
        self.process = subprocess.Popen(
            [str(launcher)],
            stdin=subprocess.DEVNULL,
            stdout=None,
            stderr=None,
            text=True,
            start_new_session=True,
        )
        _public_log("renderer_started")
        return True

    def stop(self, reason: str) -> bool:
        if not self.running:
            self.process = None
            return False
        assert self.process is not None
        try:
            os.killpg(self.process.pid, signal.SIGTERM)
            self.process.wait(timeout=4.0)
        except (ProcessLookupError, subprocess.TimeoutExpired):
            try:
                os.killpg(self.process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            try:
                self.process.wait(timeout=1.0)
            except subprocess.TimeoutExpired:
                pass
        finally:
            self.process = None
        _public_log("renderer_stopped", reason=reason)
        return True


def status_snapshot() -> dict[str, object]:
    result: dict[str, object] = {
        "component": "skeleton-generative-saver",
        "desktop_contract": "lightdm_openbox_x11",
        "idle_threshold_seconds": IDLE_SECONDS,
        "post_media_grace_seconds": POST_MEDIA_GRACE_SECONDS,
        "media_ownership": media_ownership(),
        "launcher_ready": _launcher_path().is_file() and os.access(_launcher_path(), os.X_OK),
        "display_env_present": bool(os.environ.get("DISPLAY")),
    }
    try:
        result["x11_idle_ms"] = x11_idle_ms()
        result["x11_idle_available"] = True
    except Exception:
        result["x11_idle_available"] = False
    try:
        result["locked"] = session_locked()
        result["logind_lock_available"] = True
    except Exception:
        result["logind_lock_available"] = False
    return result


def run_loop() -> int:
    renderer = RendererProcess()
    stopping = False
    resume_after = time.monotonic() + 3.0
    retry_after = 0.0
    previous_media = "UNKNOWN"
    previous_locked = True

    def request_stop(signum: int, _frame: object) -> None:
        nonlocal stopping
        stopping = True
        _public_log("shutdown_requested", signal=signum)

    signal.signal(signal.SIGTERM, request_stop)
    signal.signal(signal.SIGINT, request_stop)
    _public_log(
        "controller_started",
        desktop_contract="lightdm_openbox_x11",
        idle_threshold_seconds=IDLE_SECONDS,
        post_media_grace_seconds=POST_MEDIA_GRACE_SECONDS,
    )

    while not stopping:
        now = time.monotonic()
        try:
            locked = session_locked()
            idle_ms = x11_idle_ms()
        except Exception as exc:
            renderer.stop("desktop_state_unknown")
            _public_log("state_unavailable", reason=str(exc))
            time.sleep(max(POLL_SECONDS, 2.0))
            continue

        media = media_ownership()
        if previous_media == "OWNER" and media == "CLEAR":
            resume_after = max(resume_after, now + POST_MEDIA_GRACE_SECONDS)
            _public_log("media_released", grace_seconds=POST_MEDIA_GRACE_SECONDS)
        if previous_locked and not locked:
            resume_after = max(resume_after, now + min(IDLE_SECONDS, 30.0))
        previous_media = media
        previous_locked = locked

        if renderer.process is not None and renderer.process.poll() is not None:
            exit_code = renderer.process.returncode
            renderer.process = None
            retry_after = now + RESTART_BACKOFF_SECONDS
            _public_log("renderer_exited", exit_code=exit_code, retry_in_seconds=RESTART_BACKOFF_SECONDS)

        reason: str | None = None
        if locked:
            reason = "session_locked"
        elif media == "OWNER":
            reason = "media_owner"
        elif media == "UNKNOWN":
            reason = "media_state_unknown"
        elif idle_ms < int(IDLE_SECONDS * 1000):
            reason = "user_active"
        elif now < resume_after:
            reason = "grace_period"
        elif now < retry_after:
            reason = "restart_backoff"

        if reason is not None:
            renderer.stop(reason)
        elif not renderer.running:
            try:
                renderer.start()
            except Exception as exc:
                retry_after = now + RESTART_BACKOFF_SECONDS
                _public_log("renderer_start_failed", reason=str(exc), retry_in_seconds=RESTART_BACKOFF_SECONDS)

        time.sleep(POLL_SECONDS)

    renderer.stop("controller_shutdown")
    _public_log("controller_stopped")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--status", action="store_true", help="print bounded current integration status")
    args = parser.parse_args()
    if args.status:
        print(json.dumps(status_snapshot(), sort_keys=True, separators=(",", ":")))
        return 0
    return run_loop()


if __name__ == "__main__":
    sys.exit(main())
