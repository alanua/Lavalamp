import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import test from 'node:test';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const prepare = fs.readFileSync(path.join(root, 'scripts/prepare-wled.ps1'), 'utf8');
const readme = fs.readFileSync(path.join(root, 'README.md'), 'utf8');
const platformio = fs.readFileSync(path.join(root, 'overlays/wled/platformio_override.ini'), 'utf8');

test('WLED workspace preparation is pinned to WLED 16', () => {
  assert.match(prepare, /\[string\]\$WledVersion = "v16\.0\.0"/);
  assert.match(readme, /official WLED `v16\.0\.0`/);
  assert.match(readme, /pinned to WLED `v16\.0\.0`/);
});

test('WLED 16 path does not require patching usermods_list.cpp', () => {
  assert.match(prepare, /\$WledVersion -notmatch "\^v\?16"/);
  assert.match(platformio, /custom_usermods =\n\s+cylinder_lava/);
  assert.match(prepare, /"name": "cylinder_lava"/);
  assert.match(prepare, /"libArchive": false/);
  assert.match(prepare, /cylinder_lava\.cpp/);
  assert.match(prepare, /#include "usermod_cylinder_lava\.h"/);
});
