import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import test from 'node:test';
const root = path.resolve(import.meta.dirname,'..');
const read = file => fs.readFileSync(path.join(root,file),'utf8');
test('only stable WLED v16.0.1 exact release commit is accepted before overlays', () => {
  const script=read('scripts/prepare-wled.ps1');
  assert.match(script,/\$WledVersion = "v16\.0\.1"/);
  assert.match(script,/\$WledCommit = "29b389df1c1aaec6ff53aea742d17063b985906c"/);
  assert.match(script,/\$actualCommit -ne \$WledCommit/);
  assert.ok(script.indexOf('$actualCommit -ne $WledCommit')<script.indexOf('Copy-Item'));
  assert.match(script,/Only stable WLED v16\.0\.1 is supported/);
  for(const patch of ['wled-usermods-list-cylinder-lava.patch','wled-native-2d-cylinder-adapter.patch'])
    assert.ok(script.includes(patch));
  assert.match(script,/git apply --reverse --check/);
  assert.match(script,/git apply --check/);
  const ini=read('overlays/wled/platformio_override.ini');
  assert.match(ini,/esp32_idf_V4\.platform/);
  assert.match(ini,/esp32_idf_V4\.lib_deps/);
  assert.match(read('patches/wled-usermods-list-cylinder-lava.patch'),/wled00\/usermod.cpp/);
  assert.match(read('overlays/wled/usermods/cylinder_lava/usermod_cylinder_lava.h'),/REGISTER_USERMOD/);
});
