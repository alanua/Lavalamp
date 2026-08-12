#!/usr/bin/env bash
set -euo pipefail

export DISPLAY="${DISPLAY:-:0}"
export XAUTHORITY="${XAUTHORITY:-$HOME/.Xauthority}"
ROOT="${SKELETON_GENERATIVE_ROOT:-$HOME/.local/lib/lavalamp-home-edge}"
PORT="${SKELETON_SCREENSAVER_PORT:-8765}"
RUNTIME_ROOT="${XDG_RUNTIME_DIR:-/run/user/$(id -u)}/skeleton-generative-saver"
PROFILE="$RUNTIME_ROOT/chrome-profile"
URL="http://127.0.0.1:${PORT}/home_edge/generative_visuals/?rotate=1"
LOCK="$RUNTIME_ROOT/renderer.lock"

mkdir -p "$RUNTIME_ROOT" "$PROFILE"
exec 9>"$LOCK"
if ! /usr/bin/flock -n 9; then
  printf '%s\n' '{"component":"skeleton-generative-saver","event":"duplicate_renderer_rejected"}'
  exit 0
fi

if [[ ! -f "$ROOT/home_edge/generative_visuals/index.html" ]]; then
  printf '%s\n' '{"component":"skeleton-generative-saver","event":"renderer_source_missing"}' >&2
  exit 2
fi

CHROME=""
for candidate in /usr/bin/google-chrome-stable /usr/bin/google-chrome /usr/bin/chromium /usr/bin/chromium-browser; do
  if [[ -x "$candidate" ]]; then CHROME="$candidate"; break; fi
done
if [[ -z "$CHROME" ]]; then
  printf '%s\n' '{"component":"skeleton-generative-saver","event":"chromium_missing"}' >&2
  exit 3
fi

/usr/bin/python3 -m http.server "$PORT" --bind 127.0.0.1 --directory "$ROOT" >/dev/null 2>&1 &
SERVER_PID=$!
cleanup() {
  kill "$SERVER_PID" 2>/dev/null || true
  wait "$SERVER_PID" 2>/dev/null || true
}
trap cleanup EXIT INT TERM HUP

ready=0
for _ in $(seq 1 40); do
  if /usr/bin/python3 - "$PORT" <<'PY' >/dev/null 2>&1
import sys, urllib.request
port=int(sys.argv[1])
with urllib.request.urlopen(f'http://127.0.0.1:{port}/home_edge/generative_visuals/index.html', timeout=.25) as response:
    raise SystemExit(0 if response.status == 200 else 1)
PY
  then ready=1; break; fi
  sleep .05
done
if [[ "$ready" -ne 1 ]]; then
  printf '%s\n' '{"component":"skeleton-generative-saver","event":"local_server_unready"}' >&2
  exit 4
fi

"$CHROME" \
  --kiosk \
  --app="$URL" \
  --user-data-dir="$PROFILE" \
  --incognito \
  --no-first-run \
  --no-default-browser-check \
  --disable-sync \
  --disable-background-networking \
  --disable-component-update \
  --disable-default-apps \
  --disable-features=Translate,MediaRouter \
  --disable-session-crashed-bubble \
  --disable-infobars \
  --autoplay-policy=no-user-gesture-required \
  --ozone-platform=x11
