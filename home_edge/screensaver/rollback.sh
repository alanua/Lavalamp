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

if [[ -d "$BACKUP/target.previous" ]]; then mv "$BACKUP/target.previous" "$TARGET"; fi
if [[ -f "$BACKUP/unit.previous" ]]; then mv "$BACKUP/unit.previous" "$UNIT"; fi
if [[ -f "$BACKUP/env.previous" ]]; then
  mv "$BACKUP/env.previous" "$ENV_FILE"
elif [[ -e "$BACKUP/captured" ]]; then
  rm -f "$ENV_FILE"
fi

/usr/bin/systemctl --user daemon-reload
if [[ -f "$UNIT" ]]; then
  if [[ "$(cat "$BACKUP/unit_was_enabled" 2>/dev/null || echo no)" == yes ]]; then
    /usr/bin/systemctl --user enable skeleton-generative-saver.service >/dev/null
  else
    /usr/bin/systemctl --user disable skeleton-generative-saver.service >/dev/null 2>&1 || true
  fi
  if [[ "$(cat "$BACKUP/unit_was_active" 2>/dev/null || echo no)" == yes ]]; then
    /usr/bin/systemctl --user start skeleton-generative-saver.service
  else
    /usr/bin/systemctl --user stop skeleton-generative-saver.service >/dev/null 2>&1 || true
  fi
fi

rm -f "$BACKUP/captured" "$BACKUP/unit_was_enabled" "$BACKUP/unit_was_active"

printf 'rollback_status=APPLIED\n'
printf 'lock_policy_changed=false\n'
printf 'power_policy_changed=false\n'
printf 'stock_packages_changed=false\n'
