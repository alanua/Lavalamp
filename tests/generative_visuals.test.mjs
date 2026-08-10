import assert from 'node:assert/strict';
import test from 'node:test';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { SCENES, deterministicSceneState } from '../home_edge/generative_visuals/scenes.mjs';
import { makeSceneFrame, validateSceneFrame } from '../home_edge/generative_visuals/scene-bus.mjs';

const here = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(here, '..');

test('all thirteen required scenes are unique and selectable', () => {
  const required = ['infinite_layers','organic_sheet','topo_flow','porous_sculpture','reaction_diffusion','metaball_tunnel','prism_bloom','spectral_flame','accretion_horizon','particle_veil','chromatic_glass','volumetric_loom','field_lines'];
  assert.equal(SCENES.length, 13);
  assert.deepEqual(SCENES.map((s) => s.id), required);
  assert.equal(new Set(SCENES.map((s) => s.id)).size, 13);
});

test('same seed and timestamp produce deterministic scene metadata', () => {
  for (const scene of SCENES) {
    const a = deterministicSceneState(scene.id, 123456, 987654321);
    const b = deterministicSceneState(scene.id, 123456, 987654321);
    assert.deepEqual(a, b);
  }
});

test('scene bus payload validates required schema and ranges', () => {
  for (const scene of SCENES) {
    const frame = makeSceneFrame(deterministicSceneState(scene.id, 7, 123456));
    assert.equal(frame.schema_version, 'lavalamp.scene_frame.v1');
    assert.equal(frame.source, 'generative');
    assert.ok(validateSceneFrame(frame));
    assert.ok(frame.palette.length >= 2 && frame.palette.length <= 5);
  }
});

test('invalid scene bus values fail closed', () => {
  const frame = makeSceneFrame(deterministicSceneState(SCENES[0].id, 3, 1));
  assert.equal(validateSceneFrame({ ...frame, brightness: 1.1 }), false);
  assert.equal(validateSceneFrame({ ...frame, direction: { x: -2, y: 0 } }), false);
  assert.equal(validateSceneFrame({ ...frame, palette: [[0, 0, 0]] }), false);
});

test('runtime source contains no remote URLs or network APIs', () => {
  const dir = path.join(root, 'home_edge', 'generative_visuals');
  for (const name of fs.readdirSync(dir)) {
    const full = path.join(dir, name);
    if (!fs.statSync(full).isFile()) continue;
    if (name.endsWith('.md')) continue;
    const text = fs.readFileSync(full, 'utf8');
    assert.doesNotMatch(text, /https?:\/\//i, name);
    assert.doesNotMatch(text, /\b(fetch|XMLHttpRequest|WebSocket|EventSource)\s*\(/, name);
  }
});
