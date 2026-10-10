import assert from 'node:assert/strict';
import test from 'node:test';
import fs from 'node:fs';
import os from 'node:os';
import { spawnSync } from 'node:child_process';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { SCENES, blendSceneStates, deterministicSceneState } from '../home_edge/generative_visuals/scenes.mjs';
import { makeSceneFrame, validateSceneFrame } from '../home_edge/generative_visuals/scene-bus.mjs';

const here = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(here, '..');
const readRoot = (...parts) => fs.readFileSync(path.join(root, ...parts), 'utf8');

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
    assert.ok(state.direction.x >= -1 && state.direction.x <= 1);
    assert.ok(state.direction.y >= -1 && state.direction.y <= 1);
  }
  for (const key of ['brightness', 'energy', 'tempo', 'accent']) {
    assert.ok(Math.abs(right[key] - left[key]) < 0.03, `${key} must not jump at the scene-id handoff`);
  }
  assert.ok(Math.abs(right.direction.x - left.direction.x) < 0.05);
  assert.ok(Math.abs(right.direction.y - left.direction.y) < 0.05);
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

test('Scene Bus has independent 20 Hz scheduler and hidden/pause suppression', () => {
  const source = fs.readFileSync(path.join(root, 'home_edge', 'generative_visuals', 'app.mjs'), 'utf8');
  assert.match(source, /SCENE_BUS_INTERVAL_MS\s*=\s*50/);
  assert.match(source, /setInterval\(\(\)\s*=>\s*\{\s*if\s*\(!hidden\s*&&\s*!settings\.paused\)\s*emitBus\(\);\s*\},\s*SCENE_BUS_INTERVAL_MS\)/);
  assert.doesNotMatch(source, /requestAnimationFrame[\s\S]{0,500}lastBus/);
  assert.match(source, /blendSceneStates\(a,b,currentBlend\)/);
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
  const source = readRoot('home_edge', 'generative_visuals', 'renderer.mjs');
  assert.match(source, /float quality01\(\)/);
  assert.match(source, /metaballTunnel[\s\S]*?limit=3\.\+floor\(3\.\*quality01\(\)/);
  assert.match(source, /particleVeil[\s\S]*?layerLimit=1\.\+floor\(2\.\*quality01\(\)/);
  assert.match(source, /chromaticGlass[\s\S]*?glassLimit=3\.\+floor\(2\.\*quality01\(\)/);
});

test('cylinder geometry wraps the horizontal seam and clamps vertical sampling', () => {
  const source = readRoot('overlays', 'wled', 'usermods', 'cylinder_lava', 'cylinder_geometry.h');
  assert.match(source, /static inline uint8_t wrapX\(int16_t x, uint8_t width\)/);
  assert.match(source, /while \(x < 0\) x \+= width;/);
  assert.match(source, /while \(x >= width\) x -= width;/);
  assert.match(source, /sampleWrapped[\s\S]*indexOf\(wrapX\(x, surface\.width\), clampY\(y, surface\.height\), surface\)/);

  const surface = { width: 8, height: 4 };
  const field = Array.from({ length: surface.width * surface.height }, (_, index) => {
    const x = index % surface.width;
    const y = Math.floor(index / surface.width);
    return y * 10 + x;
  });
  const indexOf = (x, y) => y * surface.width + x;
  const wrapX = (x) => {
    while (x < 0) x += surface.width;
    while (x >= surface.width) x -= surface.width;
    return x;
  };
  const clampY = (y) => Math.max(0, Math.min(surface.height - 1, y));
  const sampleWrapped = (x, y) => field[indexOf(wrapX(x), clampY(y))];

  assert.equal(sampleWrapped(-1, 1), field[indexOf(7, 1)]);
  assert.equal(sampleWrapped(8, 1), field[indexOf(0, 1)]);
  assert.equal(sampleWrapped(17, -2), field[indexOf(1, 0)]);
  assert.equal(sampleWrapped(-10, 99), field[indexOf(6, 3)]);
});

test('cylinder y orientation is top-high and bottom-low for height-from-bottom math', () => {
  const geometry = readRoot('overlays', 'wled', 'usermods', 'cylinder_lava', 'cylinder_geometry.h');
  const lava = readRoot('overlays', 'wled', 'usermods', 'cylinder_lava', 'lava_scene.h');
  const flame = readRoot('overlays', 'wled', 'usermods', 'cylinder_lava', 'flame_scene.h');
  assert.match(geometry, /heightFromBottom8[\s\S]*height - 1 - y/);
  assert.match(lava, /const uint8_t h = heightFromBottom8\(y, context\.surface\.height\);[\s\S]*255 - h/);
  assert.match(flame, /const uint8_t h = heightFromBottom8\(y, context\.surface\.height\);[\s\S]*255 - h/);

  const heightFromBottom8 = (y, height) => {
    if (height <= 1) return 0;
    return Math.floor((((height - 1 - y) * 255) + Math.floor((height - 1) / 2)) / (height - 1));
  };
  const height = 9;
  const values = Array.from({ length: height }, (_, y) => heightFromBottom8(y, height));

  assert.equal(values[0], 255);
  assert.equal(values[height - 1], 0);
  assert.equal(values[4], 128);
  for (let y = 1; y < values.length; y++) {
    assert.ok(values[y] < values[y - 1], `height-from-bottom must descend as y increases: ${values}`);
  }
});

test('radial falloff measures shortest cylindrical distance across the wrap seam', () => {
  const source = readRoot('overlays', 'wled', 'usermods', 'cylinder_lava', 'cylinder_geometry.h');
  assert.match(source, /shortestXDeltaQ8\(xToQ8\(x\), centerX, surface\)/);
  assert.match(source, /if \(dx > int32_t\(circumference \/ 2U\)\) dx -= circumference;/);
  assert.match(source, /if \(dx < -int32_t\(circumference \/ 2U\)\) dx \+= circumference;/);

  const surface = { width: 16 };
  const xToQ8 = (x) => (x << 8) + 128;
  const shortestXDeltaQ8 = (a, b) => {
    const circumference = surface.width << 8;
    let dx = a - b;
    if (dx > circumference / 2) dx -= circumference;
    if (dx < -circumference / 2) dx += circumference;
    return dx;
  };
  const center = xToQ8(0);

  assert.equal(Math.abs(shortestXDeltaQ8(xToQ8(15), center)), 256);
  assert.equal(Math.abs(shortestXDeltaQ8(xToQ8(1), center)), 256);
  assert.equal(Math.abs(shortestXDeltaQ8(xToQ8(8), center)), 2048);
  assert.ok(Math.abs(shortestXDeltaQ8(xToQ8(15), center)) < Math.abs(shortestXDeltaQ8(xToQ8(8), center)));
});

test('radial cylinder coordinates map x onto a periodic theta plane', () => {
  const source = readRoot('overlays', 'wled', 'usermods', 'cylinder_lava', 'cylinder_volume.h');
  assert.match(source, /coord\.theta = CY_TWO_PI \* \(float\(x\) \/ float\(surface\.width\)\)/);
  assert.match(source, /coord\.h = surface\.height <= 1 \? 0\.0f : float\(y\) \/ float\(surface\.height - 1\)/);
  assert.match(source, /cyCylinderNoise[\s\S]*cosf\(theta\) \* r \* sx[\s\S]*sinf\(theta\) \* r \* sz/);

  const width = 12;
  const height = 5;
  const twoPi = Math.PI * 2;
  const cyCoord = (x, y, sample) => ({
    theta: twoPi * (x / width),
    h: height <= 1 ? 0 : y / (height - 1),
    r: 1 - sample / 3,
  });
  const radialPlane = ({ theta, h, r }, sx = 2, sy = 4, sz = 2) => ({
    x: Math.cos(theta) * r * sx,
    y: h * sy,
    z: Math.sin(theta) * r * sz,
  });

  assert.equal(cyCoord(0, 0, 0).h, 0);
  assert.equal(cyCoord(0, height - 1, 0).h, 1);
  const seamStart = radialPlane(cyCoord(0, 2, 0));
  const seamEnd = radialPlane({ ...cyCoord(width, 2, 0), theta: twoPi });
  assert.ok(Math.abs(seamStart.x - seamEnd.x) < 1e-12);
  assert.ok(Math.abs(seamStart.y - seamEnd.y) < 1e-12);
  assert.ok(Math.abs(seamStart.z - seamEnd.z) < 1e-12);
});

test('renderer and app avoid random palette shifts or strobe in the frame path', () => {
  const renderer = readRoot('home_edge', 'generative_visuals', 'renderer.mjs');
  const app = readRoot('home_edge', 'generative_visuals', 'app.mjs');
  const frameBody = app.match(/function frame\(now\) \{[\s\S]*?\n\}/)?.[0] || '';
  const emitBusBody = app.match(/function emitBus\(\) \{[^\n]*\}/)?.[0] || '';

  assert.doesNotMatch(renderer, /Math\.random|crypto\.getRandomValues/);
  assert.doesNotMatch(frameBody, /Math\.random|crypto\.getRandomValues/);
  assert.doesNotMatch(emitBusBody, /Math\.random|crypto\.getRandomValues/);
  assert.match(app, /renderer\.setPalette\(blendPalettes\(paletteFor\(sceneA\), paletteFor\(sceneB\), currentBlend\)\)/);
  assert.match(app, /document\.querySelector\('#randomize'\)\.addEventListener\('click',\(\)=>\{settings\.seed=normalizeSeed\(crypto\.getRandomValues/);
  assert.match(renderer, /vec3 pal\(float x\)[\s\S]*mix\(uPalette0,uPalette1[\s\S]*mix\(uPalette3,uPalette4/);
  assert.match(renderer, /render\(timeSeconds\)[\s\S]*for\(let i=0;i<5;i\+\+\)[\s\S]*gl\.uniform3f\(l\['uPalette'\+i\],c\[0\],c\[1\],c\[2\]\)/);
  assert.doesNotMatch(renderer, /\bstrobe\b|flash\s*\(|blink\s*\(/i);
});


test('compiled C++ cylinder geometry exercises real wrap, orientation and radial seam', (t) => {
  const compiler = spawnSync('g++', ['--version'], { encoding: 'utf8' });
  if (compiler.error?.code === 'ENOENT') {
    t.skip('g++ not available for native geometry test');
    return;
  }
  assert.equal(compiler.status, 0, compiler.stderr || compiler.error?.message);

  const temp = fs.mkdtempSync(path.join(os.tmpdir(), 'lavalamp-native-geometry-'));
  try {
    // No Arduino/WLED runtime, device output, flash, or production source mutation:
    // stub only the external WLED header dependencies; compile the real geometry header.
    fs.writeFileSync(path.join(temp, 'wled.h'), [
      '#pragma once',
      '#include <cstdint>',
      '#include <cstdlib>',
      'static inline uint8_t ease8InOutApprox(uint8_t value) { return value; }',
      '',
    ].join('\n'));
    fs.writeFileSync(path.join(temp, 'FX.h'), '#pragma once\n');
    const harness = String.raw`
#include "overlays/wled/usermods/cylinder_lava/cylinder_geometry.h"
#include <cassert>
#include <cstdint>

int main() {
  using namespace CylinderLamp;
  const Surface surface{16, 9, 144};
  assert(indexOf(15, 8, surface) == 143);
  assert(wrapX(-1, surface.width) == 15);
  assert(wrapX(16, surface.width) == 0);
  assert(wrapX(33, surface.width) == 1);
  assert(clampY(-2, surface.height) == 0);
  assert(clampY(99, surface.height) == 8);
  assert(angle8(0, 16) == 0);
  assert(angle8(8, 16) == 128);
  assert(heightFromBottom8(0, 9) == 255);
  assert(heightFromBottom8(4, 9) == 128);
  assert(heightFromBottom8(8, 9) == 0);

  uint8_t pixels[144]{};
  for (unsigned i = 0; i < 144; ++i) pixels[i] = uint8_t(i & 255);
  assert(sampleWrapped(pixels, -1, 1, surface) == pixels[indexOf(15, 1, surface)]);
  assert(sampleWrapped(pixels, 16, 1, surface) == pixels[indexOf(0, 1, surface)]);
  assert(sampleWrapped(pixels, 17, -5, surface) == pixels[indexOf(1, 0, surface)]);
  assert(sampleWrapped(pixels, -2, 99, surface) == pixels[indexOf(14, 8, surface)]);

  const auto centerX = xToQ8(0);
  const auto centerY = yToQ8(4);
  assert(shortestXDeltaQ8(xToQ8(15), centerX, surface) == -256);
  assert(shortestXDeltaQ8(xToQ8(1), centerX, surface) == 256);
  const auto left = radialFalloffQ8(15, 4, surface, centerX, centerY, 512, 128);
  const auto right = radialFalloffQ8(1, 4, surface, centerX, centerY, 512, 128);
  const auto far = radialFalloffQ8(8, 4, surface, centerX, centerY, 512, 128);
  assert(left == right);
  assert(left > far);
  return 0;
}
`;
    const source = path.join(temp, 'geometry_test.cpp');
    const executable = path.join(temp, 'geometry_test');
    fs.writeFileSync(source, harness);
    const build = spawnSync('g++', [
      '-std=c++17', '-O0', '-Wall', '-Wextra',
      '-I', temp, '-I', root, source, '-o', executable,
    ], { encoding: 'utf8', timeout: 20000 });
    assert.equal(build.status, 0, build.stderr || build.error?.message);
    const run = spawnSync(executable, [], { encoding: 'utf8', timeout: 10000 });
    assert.equal(run.status, 0, run.stderr || run.error?.message);
  } finally {
    fs.rmSync(temp, { recursive: true, force: true });
  }
});
