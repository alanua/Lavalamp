import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import test from 'node:test';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const dir = path.join(root, 'overlays/wled/usermods/cylinder_lava');
const usermod = fs.readFileSync(path.join(dir, 'usermod_cylinder_lava.h'), 'utf8');
const effects = fs.readFileSync(path.join(dir, 'cy_effects_scene.h'), 'utf8');
const contract = fs.readFileSync(path.join(dir, 'cylinder_scene_contract.h'), 'utf8');
const volume = fs.readFileSync(path.join(dir, 'cylinder_volume.h'), 'utf8');
const geometry = fs.readFileSync(path.join(dir, 'cylinder_geometry.h'), 'utf8');

const expectedNames = [
  'CY Tidal Bloom',
  'CY Flame',
  'CY Lava Flow',
  'CY Ocean Drift',
  'CY Aurora',
  'CY Nebula',
  'CY Energy Pulse',
  'CY Bubbles',
  'CY Particle Storm',
  'CY Rainbow Spiral',
  'CY Matrix Rain',
  'CY Electric Storm',
  'CY Heartbeat',
  'CY Dreamscape',
  'CY Anemone',
  'CY Ripple Rings',
  'CY Comet Trails',
];

const expectedModes = [
  'mode_cy_tidal_bloom',
  'mode_cy_flame',
  'mode_cy_lava_flow',
  'mode_cy_ocean_drift',
  'mode_cy_aurora',
  'mode_cy_nebula',
  'mode_cy_energy_pulse',
  'mode_cy_bubbles',
  'mode_cy_particle_storm',
  'mode_cy_rainbow_spiral',
  'mode_cy_matrix_rain',
  'mode_cy_electric_storm',
  'mode_cy_heartbeat',
  'mode_cy_dreamscape',
  'mode_cy_anemone',
  'mode_cy_ripple_rings',
  'mode_cy_comet_trails',
];

const expectedKinds = [
  'CY_EFFECT_TIDAL_BLOOM',
  'CY_EFFECT_FLAME',
  'CY_EFFECT_LAVA_FLOW',
  'CY_EFFECT_OCEAN_DRIFT',
  'CY_EFFECT_AURORA',
  'CY_EFFECT_NEBULA',
  'CY_EFFECT_ENERGY_PULSE',
  'CY_EFFECT_BUBBLES',
  'CY_EFFECT_PARTICLE_STORM',
  'CY_EFFECT_RAINBOW_SPIRAL',
  'CY_EFFECT_MATRIX_RAIN',
  'CY_EFFECT_ELECTRIC_STORM',
  'CY_EFFECT_HEARTBEAT',
  'CY_EFFECT_DREAMSCAPE',
  'CY_EFFECT_ANEMONE',
  'CY_EFFECT_RIPPLE_RINGS',
  'CY_EFFECT_COMET_TRAILS',
];

const expectedScenes = [
  'SCENE_ID_TIDAL_BLOOM',
  'SCENE_ID_FLAME',
  'SCENE_ID_LAVA_FLOW',
  'SCENE_ID_OCEAN_DRIFT',
  'SCENE_ID_AURORA',
  'SCENE_ID_NEBULA',
  'SCENE_ID_ENERGY_PULSE',
  'SCENE_ID_BUBBLES',
  'SCENE_ID_PARTICLE_STORM',
  'SCENE_ID_RAINBOW_SPIRAL',
  'SCENE_ID_MATRIX_RAIN',
  'SCENE_ID_ELECTRIC_STORM',
  'SCENE_ID_HEARTBEAT',
  'SCENE_ID_DREAMSCAPE',
  'SCENE_ID_ANEMONE',
  'SCENE_ID_RIPPLE_RINGS',
  'SCENE_ID_COMET_TRAILS',
];

const numberList = (source) => source.split(',').map((item) => item.trim()).filter(Boolean);
const enumBody = (source, name) => source.match(new RegExp(`enum ${name} : uint8_t \\{([\\s\\S]*?)\\n\\};`))?.[1] ?? '';

test('CY Effects v2.0 exposes exactly the required 17 names in order', () => {
  const registrations = [...usermod.matchAll(/strip\.addEffect\(255,\s*&(\w+),\s*(_data_FX_MODE_CY_[A-Z0-9_]+)\);/g)];
  assert.equal(registrations.length, 17);
  assert.deepEqual(registrations.map((match) => match[1]), expectedModes);

  const names = registrations.map(([, , dataName]) => {
    const escaped = dataName.replaceAll('_', '\\_');
    const pattern = new RegExp(`static const char ${escaped}\\[\\] PROGMEM =\\n\\s+"([^"@]+)@`);
    return usermod.match(pattern)?.[1];
  });
  assert.deepEqual(names, expectedNames);
  assert.equal(new Set(names).size, 17);
});

test('obsolete exposed legacy CY names are removed', () => {
  const obsolete = [
    'CY Lava Lamp',
    'CY Plasma Core',
    'CY Deep Noise',
    'CY Aurora Tube',
    'CY Inner Swirl',
    'CY Bubbles Volume',
    'CY Ring Ripples Rainbow',
    'CY Bottom Rays',
    'CY Rising Bands',
    'CY Helical Plasma',
    'CY Noise Waves Tube',
    'CY Cell Membrane Flow',
    'CY Cross Bands Tube',
  ];
  for (const name of obsolete) {
    assert.doesNotMatch(usermod, new RegExp(`"${name.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')}@`));
  }
});

test('effect kinds and scene ids have the same exact 17-order contract', () => {
  const kinds = numberList(enumBody(effects, 'CyEffectKind')).map((line) => line.replace(/\s*=\s*\d+/, ''));
  assert.deepEqual(kinds, expectedKinds);

  const scenes = numberList(enumBody(contract, 'SceneId')).map((line) => line.replace(/\s*=\s*\d+/, ''));
  assert.deepEqual(scenes, expectedScenes);

  const sceneValues = numberList(enumBody(contract, 'SceneId')).map((line) => Number(line.match(/=\s*(\d+)/)?.[1]));
  assert.deepEqual(sceneValues, Array.from({ length: 17 }, (_, index) => index + 1));
});

test('all 17 modes route to unique scenes and unique renderer functions', () => {
  const routes = [...usermod.matchAll(/^CY_MODE\((\w+),\s*(SCENE_ID_\w+),\s*(renderCy\w+)\)$/gm)];
  assert.equal(routes.length, 17);
  assert.deepEqual(routes.map((match) => `mode_cy_${match[1]}`), expectedModes);
  assert.deepEqual(routes.map((match) => match[2]), expectedScenes);
  assert.equal(new Set(routes.map((match) => match[2])).size, 17);
  assert.equal(new Set(routes.map((match) => match[3])).size, 17);
});

test('all 17 effects dispatch to distinct field implementations, never fall-through aliases', () => {
  const cases = [...effects.matchAll(/case (CY_EFFECT_[A-Z0-9_]+): return (cyField\w+)\(c, t\);/g)];
  assert.equal(cases.length, 17);
  const map = new Map(cases.map((match) => [match[1], match[2]]));
  assert.deepEqual([...map.keys()], expectedKinds);
  assert.equal(new Set([...map.values()]).size, 17);

  for (const field of map.values()) {
    assert.match(effects, new RegExp(`static float ${field}\\(const CyCoord& c, float t\\)`));
  }
});

test('native cylindrical coordinates preserve seamless circumference and physical bottom', () => {
  assert.match(volume, /coord\.theta = CY_TWO_PI \* \(\(float\(x\) \+ 0\.5f\) \/ float\(surface\.width\)\);/);
  assert.match(volume, /coord\.h = surface\.height <= 1 \? 0\.0f : float\(surface\.height - 1 - y\) \/ float\(surface\.height - 1\);/);
  assert.match(geometry, /while \(x < 0\) x \+= width;/);
  assert.match(geometry, /while \(x >= width\) x -= width;/);
  assert.match(geometry, /return field\[indexOf\(wrapX\(x, surface\.width\), clampY\(y, surface\.height\), surface\)\];/);

  const width = 24;
  const height = 10;
  const coord = (x, y) => ({
    theta: Math.PI * 2 * ((x + 0.5) / width),
    h: (height - 1 - y) / (height - 1),
  });
  assert.equal(coord(0, height - 1).h, 0);
  assert.equal(coord(0, 0).h, 1);
  assert.ok(coord(width - 1, 4).theta < Math.PI * 2);
  assert.ok(coord(0, 4).theta > 0);
});

test('representative new fields are bounded, non-black and numerically distinct', () => {
  const gauss = (value, center, width) => Math.exp(-(((value - center) / width) ** 2));
  const fract = (value) => value - Math.floor(value);
  const wrap = (a) => {
    while (a > Math.PI) a -= Math.PI * 2;
    while (a < -Math.PI) a += Math.PI * 2;
    return a;
  };
  const angleGauss = (theta, center, width) => gauss(wrap(theta - center), 0, width);

  const lava = (theta, h, r, t) => {
    let blobs = 0;
    for (let i = 0; i < 4; i++) {
      const centerTheta = (1.47 * i) + (0.24 * Math.sin((0.00015 * t) + (1.3 * i)));
      const centerH = 0.16 + (0.70 * fract((0.000036 * t) + (0.27 * i)));
      blobs += angleGauss(theta, centerTheta, 0.38 + (0.04 * i))
        * gauss(h, centerH, 0.13 + (0.015 * i))
        * gauss(r, 0.66, 0.25);
    }
    return (1.18 * Math.exp(-h / 0.10) * gauss(r, 0.72, 0.28)) + blobs;
  };
  const heartbeat = (theta, h, r, t) => {
    const phase = (0.0012 * t) % (Math.PI * 2);
    const beat = Math.min(1.4, gauss(Math.sin(phase), 0.92, 0.18)
      + (0.72 * gauss(Math.sin(phase + 0.72), 0.92, 0.20)));
    return (0.12 + (1.15 * beat * gauss(h, 0.48 + (0.08 * beat), 0.075 + (0.025 * beat))))
      * gauss(r, 0.58, 0.30);
  };
  const comet = (theta, h, r, t) => {
    let total = 0;
    for (let i = 0; i < 3; i++) {
      const headTheta = fract((0.000047 * t) + (0.333 * i)) * Math.PI * 2;
      const headH = 0.24 + (0.25 * i) + (0.07 * Math.sin((0.00019 * t) + i));
      let d = wrap(theta - headTheta);
      if (d > 0) d -= Math.PI * 2;
      const head = gauss(d, 0, 0.16) * gauss(h, headH, 0.075);
      const trail = d <= 0 && d > -1.65
        ? Math.exp(d / 0.55) * gauss(h, headH + (0.035 * Math.sin(5 * d)), 0.095)
        : 0;
      total += (1.18 * head) + (0.48 * trail);
    }
    return total * gauss(r, 0.80, 0.18);
  };

  const values = [];
  let lit = 0;
  for (let y = 0; y < 10; y++) {
    const h = (9 - y) / 9;
    for (let x = 0; x < 24; x++) {
      const theta = Math.PI * 2 * ((x + 0.5) / 24);
      const sample = [
        lava(theta, h, 0.72, 4500),
        heartbeat(theta, h, 0.58, 4500),
        comet(theta, h, 0.80, 4500),
      ];
      for (const value of sample) {
        assert.ok(Number.isFinite(value));
        assert.ok(value >= 0);
        if (value > 0.04) lit += 1;
      }
      if (x === 5 && y === 5) values.push(...sample);
    }
  }
  assert.ok(lit > 50, 'representative new fields must have visible non-black coverage');
  assert.equal(new Set(values.map((value) => value.toFixed(6))).size, 3);
});

test('smooth diffuser-friendly pipeline is enabled', () => {
  assert.match(effects, /settings\.temporalAlpha = 92;/);
  assert.match(effects, /settings\.blurX = 42;/);
  assert.match(effects, /settings\.blurY = 30;/);
});
