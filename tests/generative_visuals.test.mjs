import assert from 'node:assert/strict';
import test from 'node:test';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { SCENES, blendSceneStates, deterministicSceneState } from '../home_edge/generative_visuals/scenes.mjs';
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

test('crossfade scene metadata remains valid and continuous', () => {
  const timestamp = 987654321;
  const a = deterministicSceneState('infinite_layers', 77, timestamp);
  const b = deterministicSceneState('prism_bloom', 77, timestamp);
  const left = blendSceneStates(a, b, 0.49);
  const middle = blendSceneStates(a, b, 0.5);
  const right = blendSceneStates(a, b, 0.51);

  for (const state of [left, middle, right]) {
    assert.ok(validateSceneFrame(makeSceneFrame(state)));
    assert.equal(state.timestamp_ms, timestamp);
    assert.equal(state.seed, 77);
    assert.ok(Math.abs(Math.hypot(state.direction.x, state.direction.y) - 1) < 1e-9 || Math.hypot(state.direction.x, state.direction.y) === 0);
  }
  for (const key of ['brightness', 'energy', 'tempo', 'accent']) {
    assert.ok(Math.abs(right[key] - left[key]) < 0.03, `${key} must not jump at the scene-id handoff`);
  }
  assert.equal(left.scene_id, a.scene_id);
  assert.equal(middle.scene_id, b.scene_id);
  assert.equal(right.scene_id, b.scene_id);
  assert.deepEqual(blendSceneStates(a, b, 0), a);
  assert.deepEqual(blendSceneStates(a, b, 1), b);
});

test('prototype-like palette names fall back to the scene palette', () => {
  const baseline = deterministicSceneState('prism_bloom', 9, 123456);
  assert.deepEqual(deterministicSceneState('prism_bloom', 9, 123456, 'constructor').palette, baseline.palette);
  assert.deepEqual(deterministicSceneState('prism_bloom', 9, 123456, 'toString').palette, baseline.palette);
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

test('GLSL smoothstep calls with literal edges are ordered', () => {
  const source = fs.readFileSync(path.join(root, 'home_edge', 'generative_visuals', 'renderer.mjs'), 'utf8');
  const literalSmoothstep = /smoothstep\(\s*(-?(?:\d+(?:\.\d*)?|\.\d+))\s*,\s*(-?(?:\d+(?:\.\d*)?|\.\d+))\s*,/g;
  const matches = [...source.matchAll(literalSmoothstep)];
  assert.ok(matches.length > 0);
  for (const match of matches) {
    const low = Number(match[1]);
    const high = Number(match[2]);
    assert.ok(low < high, `smoothstep edges must be ascending: ${match[0]}`);
  }
});

test('expensive shader scenes reduce iteration counts with uComplexity', () => {
  const source = fs.readFileSync(path.join(root, 'home_edge', 'generative_visuals', 'renderer.mjs'), 'utf8');
  assert.match(source, /float quality01\(\)/);
  assert.match(source, /metaballTunnel[\s\S]*?limit=3\.\+floor\(3\.\*quality01\(\)/);
  assert.match(source, /particleVeil[\s\S]*?layerLimit=1\.\+floor\(2\.\*quality01\(\)/);
  assert.match(source, /chromaticGlass[\s\S]*?glassLimit=3\.\+floor\(2\.\*quality01\(\)/);
});
