import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import test from 'node:test';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const dir = path.join(root, 'home_edge', 'screensaver');
const read = (name) => fs.readFileSync(path.join(dir, name), 'utf8');

test('controller is GNOME idle/lock state based and media-origin agnostic', () => {
  const source = read('skeleton-generative-saver.py');
  assert.match(source, /org\.gnome\.Mutter\.IdleMonitor/);
  assert.match(source, /org\.gnome\.ScreenSaver/);
  assert.match(source, /def media_ownership\(\)/);
  assert.match(source, /return "OWNER"/);
  assert.match(source, /return "CLEAR"/);
  assert.match(source, /return "UNKNOWN"/);
  assert.match(source, /reason = "media_owner"/);
  assert.doesNotMatch(source, /youtube|chromecast|\bmpv\b/i);
});

test('unknown media state fails closed and media release has bounded grace', () => {
  const source = read('skeleton-generative-saver.py');
  assert.match(source, /elif media == "UNKNOWN":\s*\n\s*reason = "media_state_unknown"/);
  assert.match(source, /previous_media == "OWNER" and media == "CLEAR"/);
  assert.match(source, /POST_MEDIA_GRACE_SECONDS/);
  assert.match(source, /resume_after = max\(resume_after, now \+ POST_MEDIA_GRACE_SECONDS\)/);
});

test('renderer is local-only kiosk and duplicate-safe', () => {
  const source = read('launch-renderer.sh');
  assert.match(source, /http:\/\/127\.0\.0\.1:/);
  assert.match(source, /--bind 127\.0\.0\.1/);
  assert.match(source, /--kiosk/);
  assert.match(source, /flock -n/);
  assert.doesNotMatch(source, /https:\/\//);
});

test('install and rollback preserve lock/power policy and avoid package mutation', () => {
  for (const name of ['install.sh', 'rollback.sh']) {
    const source = read(name);
    assert.doesNotMatch(source, /\bsudo\b/);
    assert.doesNotMatch(source, /\bapt(?:-get)?\b|\bdpkg\s+--remove|\bpurge\b/);
    assert.doesNotMatch(source, /gsettings\s+set|dconf\s+write/);
    assert.match(source, /lock_policy_changed=false/);
    assert.match(source, /power_policy_changed=false/);
  }
});

test('systemd unit is a user-session controller with process-group cleanup', () => {
  const unit = read('skeleton-generative-saver.service');
  assert.match(unit, /After=graphical-session\.target/);
  assert.match(unit, /WantedBy=graphical-session\.target/);
  assert.match(unit, /KillMode=control-group/);
  assert.match(unit, /NoNewPrivileges=yes/);
  assert.doesNotMatch(unit, /User=root|sudo/);
});

test('status inventories optional stock visuals without mutating them', () => {
  const source = read('status.sh');
  assert.match(source, /xscreensaver/);
  assert.match(source, /mate-screensaver/);
  assert.match(source, /stock_visual_inventory_begin/);
  assert.doesNotMatch(source, /apt|purge|remove/);
});
