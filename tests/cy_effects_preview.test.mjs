import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import test from 'node:test';
const root = path.resolve(import.meta.dirname, '..');
test('all 17 production renderers: deterministic 16x16, temporal, periodic, live and distinct', () => {
  const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'cy-preview-'));
  try {
    const binary = path.join(tmp, 'preview');
    const compile = spawnSync(process.env.CXX || 'g++', ['-std=c++17', '-DCY_PREVIEW_HOST', '-I'+path.join(root,'tests'),
      '-I'+path.join(root,'overlays/wled/usermods/cylinder_lava'), path.join(root,'tests/cy_effects_preview_host.cpp'), '-o', binary], {encoding:'utf8'});
    assert.equal(compile.status,0,compile.stderr);
    const run = spawnSync(binary,[],{encoding:'utf8'});
    assert.equal(run.status,0,run.stdout+run.stderr);
    assert.equal(run.stdout.trim().split('\n').length,17);
    console.log(run.stdout.trim());
  } finally { fs.rmSync(tmp,{recursive:true,force:true}); }
});
