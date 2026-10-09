import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import test from 'node:test';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const dir = path.join(root, 'home_edge', 'screensaver');
const read = (name) => fs.readFileSync(path.join(dir, name), 'utf8');

test('controller matches canonical LightDM/Openbox/X11 desktop contract', () => {
  const source = read('skeleton-generative-saver.py');
  assert.match(source, /lightdm_openbox_x11/);
  assert.match(source, /XScreenSaverQueryInfo/);
  assert.match(source, /libX11/);
  assert.match(source, /libXss/);
  assert.match(source, /LockedHint/);
  assert.doesNotMatch(source, /org\.gnome\./);
});

test('controller is media-origin agnostic and fails closed on ambiguity', () => {
  const source = read('skeleton-generative-saver.py');
  assert.match(source, /def media_ownership\(\)/);
  assert.match(source, /return "OWNER"/);
  assert.match(source, /return "CLEAR"/);
  assert.match(source, /return "UNKNOWN"/);
  assert.match(source, /reason = "media_owner"/);
  assert.match(source, /reason = "media_state_unknown"/);
  assert.doesNotMatch(source, /youtube|chromecast|\bmpv\b/i);
});

test('media release has bounded grace instead of immediate saver relaunch', () => {
  const source = read('skeleton-generative-saver.py');
  assert.match(source, /previous_media == "OWNER" and media == "CLEAR"/);
  assert.match(source, /POST_MEDIA_GRACE_SECONDS/);
  assert.match(source, /resume_after = max\(resume_after, now \+ POST_MEDIA_GRACE_SECONDS\)/);
});

test('renderer is local-only X11 kiosk and duplicate-safe', () => {
  const source = read('launch-renderer.sh');
  assert.match(source, /http:\/\/127\.0\.0\.1:/);
  assert.match(source, /urllib\.request\.urlopen\(f'http:\/\/127\.0\.0\.1:\{port\}/);
  assert.match(source, /--bind 127\.0\.0\.1/);
  assert.match(source, /--kiosk/);
  assert.match(source, /--ozone-platform=x11/);
  assert.match(source, /DISPLAY="\$\{DISPLAY:-:0\}"/);
  assert.match(source, /flock -n/);
  assert.match(source, /duplicate_renderer_rejected/);
  assert.doesNotMatch(source, /https:\/\//);
  assert.doesNotMatch(source, /0\.0\.0\.0|\[::\]|localhost/);
});

test('controller starts at most one renderer and backs off duplicate churn', () => {
  const source = read('skeleton-generative-saver.py');
  assert.match(source, /def start\(self\) -> bool:\s+if self\.running:\s+return False/s);
  assert.match(source, /elif not renderer\.running:\s+try:\s+renderer\.start\(\)/s);
  assert.match(source, /renderer\.process\.poll\(\) is not None:[\s\S]*?retry_after = now \+ RESTART_BACKOFF_SECONDS/);
  assert.match(source, /elif now < retry_after:\s+reason = "restart_backoff"/);
  assert.doesNotMatch(source, /while not stopping:[\s\S]{0,1500}renderer\.start\(\)[\s\S]{0,300}renderer\.start\(\)/);
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
  const install = read('install.sh');
  assert.match(install, /\/usr\/bin\/loginctl/);
  assert.match(install, /ctypes\.util\.find_library\(name\)/);
  assert.match(install, /\("Xss", "libXss\.so\.1"\)/);
});

test('status path preserves lock and media ownership as observed state only', () => {
  const source = read('skeleton-generative-saver.py');
  assert.match(source, /"media_ownership": media_ownership\(\)/);
  assert.match(source, /result\["locked"\] = session_locked\(\)/);
  assert.match(source, /logind_lock_available/);
  assert.doesNotMatch(source, /LockSession|UnlockSession|SetLockedHint/);
  assert.doesNotMatch(source, /media_ownership\(\)[\s\S]{0,400}(?:kill|terminate|stop)\(/);
});

test('systemd unit starts in normal user manager and cleans its process group', () => {
  const unit = read('skeleton-generative-saver.service');
  assert.match(unit, /WantedBy=default\.target/);
  assert.match(unit, /KillMode=control-group/);
  assert.match(unit, /NoNewPrivileges=yes/);
  assert.doesNotMatch(unit, /User=root|sudo|graphical-session\.target/);
});

test('status inventories optional stock visuals without mutating them', () => {
  const source = read('status.sh');
  assert.match(source, /xscreensaver/);
  assert.match(source, /mate-screensaver/);
  assert.match(source, /stock_visual_inventory_begin/);
  assert.doesNotMatch(source, /apt|purge|remove/);
});
