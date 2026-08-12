#!/usr/bin/env bash
set -euo pipefail

TARGET="$HOME/.local/lib/lavalamp-home-edge"
UNIT_DIR="$HOME/.config/systemd/user"
UNIT="$UNIT_DIR/skeleton-generative-saver.service"
ENV_FILE="$HOME/.config/skeleton/generative-saver.env"
STATE="$HOME/.local/state/skeleton-generative-saver"
BACKUP="$STATE/rollback"

/usr/bin/systemctl --user disable --now skeleton-generative-saver.service >/dev/null 2>&1 || true

rm -rf "$TARGET"
rm -f "$UNIT"

if [[ -d "$BACKUP/target.previous" ]]; then
  mv "$BACKUP/target.previous" "$TARGET"
fi
if [[ -f "$BACKUP/unit.previous" ]]; then
  mv "$BACKUP/unit.previous" "$UNIT"
fi
if [[ -f "$BACKUP/env.previous" ]]; then
  mv "$BACKUP/env.previous" "$ENV_FILE"
elif [[ -e "$BACKUP/captured" ]]; then
  rm -f "$ENV_FILE"
fi
rm -f "$BACKUP/captured"

/usr/bin/systemctl --user daemon-reload
if [[ -f "$UNIT" ]]; then
  /usr/bin/systemctl --user enable --now "$(basename "$UNIT")" >/dev/null
fi

printf 'rollback_status=APPLIED\n'
printf 'lock_policy_changed=false\n'
printf 'power_policy_changed=false\n'
printf 'stock_packages_changed=false\n'
