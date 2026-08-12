#!/usr/bin/env bash
set -euo pipefail

TARGET="$HOME/.local/lib/lavalamp-home-edge"
CONTROLLER="$TARGET/home_edge/screensaver/skeleton-generative-saver.py"

if /usr/bin/systemctl --user is-active --quiet skeleton-generative-saver.service; then
  echo 'service_active=true'
else
  echo 'service_active=false'
fi
if /usr/bin/systemctl --user is-enabled --quiet skeleton-generative-saver.service 2>/dev/null; then
  echo 'service_enabled=true'
else
  echo 'service_enabled=false'
fi

if [[ -x "$CONTROLLER" ]]; then
  "$CONTROLLER" --status || true
else
  echo 'controller_installed=false'
fi

echo 'stock_visual_inventory_begin'
for package in xscreensaver xscreensaver-data xscreensaver-data-extra xscreensaver-gl xscreensaver-gl-extra mate-screensaver; do
  if /usr/bin/dpkg-query -W -f='${Status}' "$package" 2>/dev/null | grep -q '^install ok installed$'; then
    printf 'optional_visual_package=%s installed=true\n' "$package"
  fi
done
for entry in "$HOME/.config/autostart/xscreensaver.desktop" "$HOME/.config/autostart/mate-screensaver.desktop"; do
  if [[ -e "$entry" ]]; then
    printf 'optional_visual_autostart=%s present=true\n' "$(basename "$entry")"
  fi
done
echo 'stock_visual_inventory_end'
echo 'gnome_lock_policy_mutated=false'
echo 'display_power_policy_mutated=false'
