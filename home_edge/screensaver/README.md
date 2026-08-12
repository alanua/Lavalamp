# Home Edge generative screensaver integration

This package installs the local `home_edge/generative_visuals` renderer as the Home Edge visual screensaver without changing authentication, session lock state, DPMS, suspend, media source, volume or playback.

## Target desktop contract

The canonical Debian media bootstrap is **LightDM + Openbox + Xorg**, not GNOME. The controller therefore uses the X11 ScreenSaver extension (through Python `ctypes` with the already-present `libX11`/`libXss` runtime libraries) to read real keyboard/mouse idle time. Session lock state is read from systemd-logind `LockedHint`. The service starts from the normal user `default.target`; it fails closed until the X display and logind session are observable.

`DISPLAY` defaults to `:0` and `XAUTHORITY` to `~/.Xauthority`, matching the canonical LightDM/Openbox target. These are fixed local defaults only; there is no network discovery.

## Runtime contract

`skeleton-generative-saver.py` starts the renderer only when all of these are true:

- the active logind session is not locked;
- X11 idle time is above `SKELETON_SCREENSAVER_IDLE_SECONDS`;
- the canonical media-display-ownership guard explicitly reports `CLEAR`;
- post-media/unlock grace and restart backoff have elapsed.

Any confirmed media owner immediately stops the renderer. An unknown media state fails closed and also stops/blocks the renderer. When media ownership releases, the normal idle policy resumes only after `SKELETON_SCREENSAVER_POST_MEDIA_GRACE_SECONDS`; the saver is not relaunched immediately on pause/stop/end.

The media guard path defaults to `~/.local/bin/home-edge-media-display-owner`. Its bounded protocol is:

- `guard status` exits `0`: confirmed video/media session owns the display;
- exits `1`: display explicitly clear for the idle saver;
- any other exit or missing/unexecutable guard: state `UNKNOWN`, saver must not start.

The guard is intentionally an integration hook rather than a second media detector. Home Edge must wire it to the canonical Skeleton Cast/player/media-session ownership adapter. It exposes only the state class through exit status; titles, URLs, accounts, session IDs and device identifiers are not consumed here.

## Renderer

`launch-renderer.sh` serves the installed package only on `127.0.0.1` and launches Chromium/Chrome in X11 kiosk app mode against the local generative renderer. A runtime `flock` rejects duplicate concurrent renderers. The controller starts the launcher in its own process group and terminates the whole group on activity, lock, media preemption or shutdown.

## Install / status / rollback

From a checked-out Lavalamp source tree:

```sh
home_edge/screensaver/install.sh
home_edge/screensaver/status.sh
home_edge/screensaver/rollback.sh
```

The installer copies only this package and the generative renderer into `~/.local/lib/lavalamp-home-edge`, installs a `systemd --user` service, and creates a private environment file only when one does not already exist. It does not use `sudo`, does not modify lock settings, does not change DPMS/suspend policy, and does not install/remove packages.

Rollback disables the new user service and restores the one captured pre-install snapshot, including its previous enabled/active state.

## Stock visual saver policy

`status.sh` inventories known optional visual-saver packages/autostart entries such as `xscreensaver*` and `mate-screensaver`. Inventory is report-only. Package removal is deliberately not embedded in this source installer: the verified target must first prove which optional saver is actually installed, then any package mutation is performed later through the canonical audited Home Edge operation with rollback. LightDM, Openbox, Xorg, authentication and power-management components are never removal candidates.

## Validation

```sh
python3 -m py_compile home_edge/screensaver/skeleton-generative-saver.py
bash -n home_edge/screensaver/launch-renderer.sh
bash -n home_edge/screensaver/install.sh
bash -n home_edge/screensaver/status.sh
bash -n home_edge/screensaver/rollback.sh
node --test tests/screensaver_integration.test.mjs
node --test tests/generative_visuals.test.mjs
git diff --check
```
