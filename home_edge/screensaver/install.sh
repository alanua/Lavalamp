#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="${1:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)}"
TARGET="$HOME/.local/lib/lavalamp-home-edge"
UNIT_DIR="$HOME/.config/systemd/user"
UNIT="$UNIT_DIR/skeleton-generative-saver.service"
ENV_DIR="$HOME/.config/skeleton"
ENV_FILE="$ENV_DIR/generative-saver.env"
STATE="$HOME/.local/state/skeleton-generative-saver"
BACKUP="$STATE/rollback"

required=(
  "$REPO_ROOT/home_edge/generative_visuals/index.html"
  "$REPO_ROOT/home_edge/screensaver/skeleton-generative-saver.py"
  "$REPO_ROOT/home_edge/screensaver/launch-renderer.sh"
  "$REPO_ROOT/home_edge/screensaver/skeleton-generative-saver.service"
)
for path in "${required[@]}"; do
  [[ -f "$path" ]] || { echo "missing_source=$path" >&2; exit 2; }
done
for cmd in /usr/bin/python3 /usr/bin/gdbus /usr/bin/systemctl /usr/bin/flock; do
  [[ -x "$cmd" ]] || { echo "missing_command=$cmd" >&2; exit 3; }
done

mkdir -p "$STATE" "$UNIT_DIR" "$ENV_DIR" "$BACKUP"
chmod 700 "$STATE" "$ENV_DIR" "$BACKUP"

# Keep exactly one rollback snapshot of the state that existed before the first apply.
if [[ ! -e "$BACKUP/captured" ]]; then
  if [[ -d "$TARGET" ]]; then
    cp -a "$TARGET" "$BACKUP/target.previous"
  fi
  if [[ -f "$UNIT" ]]; then
    cp -a "$UNIT" "$BACKUP/unit.previous"
  fi
  if [[ -f "$ENV_FILE" ]]; then
    cp -a "$ENV_FILE" "$BACKUP/env.previous"
  fi
  : > "$BACKUP/captured"
fi

STAGING="$STATE/staging.$$"
rm -rf "$STAGING"
mkdir -p "$STAGING/home_edge"
cp -a "$REPO_ROOT/home_edge/generative_visuals" "$STAGING/home_edge/"
cp -a "$REPO_ROOT/home_edge/screensaver" "$STAGING/home_edge/"
chmod 755 "$STAGING/home_edge/screensaver/skeleton-generative-saver.py" "$STAGING/home_edge/screensaver/launch-renderer.sh"

rm -rf "$TARGET.new"
mv "$STAGING" "$TARGET.new"
if [[ -d "$TARGET" ]]; then rm -rf "$TARGET.old"; mv "$TARGET" "$TARGET.old"; fi
mv "$TARGET.new" "$TARGET"
rm -rf "$TARGET.old"

install -m 0644 "$TARGET/home_edge/screensaver/skeleton-generative-saver.service" "$UNIT"
if [[ ! -f "$ENV_FILE" ]]; then
  cat > "$ENV_FILE" <<EOF
SKELETON_SCREENSAVER_IDLE_SECONDS=120
SKELETON_SCREENSAVER_POST_MEDIA_GRACE_SECONDS=120
SKELETON_SCREENSAVER_POLL_SECONDS=1
SKELETON_SCREENSAVER_MEDIA_GUARD=$HOME/.local/bin/home-edge-media-display-owner
EOF
  chmod 600 "$ENV_FILE"
fi

/usr/bin/systemctl --user daemon-reload
/usr/bin/systemctl --user enable --now skeleton-generative-saver.service

printf 'install_status=APPLIED\n'
printf 'target=%s\n' "$TARGET"
printf 'unit=skeleton-generative-saver.service\n'
printf 'lock_policy_changed=false\n'
printf 'power_policy_changed=false\n'
printf 'stock_packages_changed=false\n'
