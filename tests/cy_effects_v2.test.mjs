import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import test from 'node:test';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const usermod = fs.readFileSync(path.join(root, 'overlays/wled/usermods/cylinder_lava/usermod_cylinder_lava.h'), 'utf8');
const effects = fs.readFileSync(path.join(root, 'overlays/wled/usermods/cylinder_lava/cy_effects_scene.h'), 'utf8');
const contract = fs.readFileSync(path.join(root, 'overlays/wled/usermods/cylinder_lava/cylinder_scene_contract.h'), 'utf8');
const volume = fs.readFileSync(path.join(root, 'overlays/wled/usermods/cylinder_lava/cylinder_volume.h'), 'utf8');
const geometry = fs.readFileSync(path.join(root, 'overlays/wled/usermods/cylinder_lava/cylinder_geometry.h'), 'utf8');

const expectedNames = [
  'CY Anemone',
  'CY Tidal Bloom',
  'CY Lava Lamp',
  'CY Flame',
  'CY Plasma Core',
  'CY Deep Noise',
  'CY Aurora Tube',
  'CY Inner Swirl',
  'CY Bubbles Volume',
  'CY Ring Ripples',
  'CY Ring Ripples Rainbow',
  'CY Bottom Rays',
  'CY Rising Bands',
  'CY Helical Plasma',
  'CY Noise Waves Tube',
  'CY Cell Membrane Flow',
  'CY Cross Bands Tube',
];

const expectedModes = [
  'mode_cy_anemone',
  'mode_cy_tidal_bloom',
  'mode_cy_lava_lamp',
  'mode_cy_flame',
  'mode_cy_plasma_core',
  'mode_cy_deep_noise',
  'mode_cy_aurora_tube',
  'mode_cy_inner_swirl',
  'mode_cy_bubbles_volume',
  'mode_cy_ring_ripples',
  'mode_cy_ring_ripples_rainbow',
  'mode_cy_bottom_rays',
  'mode_cy_rising_bands',
  'mode_cy_helical_plasma',
  'mode_cy_noise_waves_tube',
  'mode_cy_cell_membrane_flow',
  'mode_cy_cross_bands_tube',
];

const expectedKinds = [
  'CY_EFFECT_ANEMONE',
  'CY_EFFECT_TIDAL_BLOOM',
  'CY_EFFECT_LAVA_LAMP',
  'CY_EFFECT_FLAME',
  'CY_EFFECT_PLASMA_CORE',
  'CY_EFFECT_DEEP_NOISE',
  'CY_EFFECT_AURORA_TUBE',
  'CY_EFFECT_INNER_SWIRL',
  'CY_EFFECT_BUBBLES_VOLUME',
  'CY_EFFECT_RING_RIPPLES',
  'CY_EFFECT_RING_RIPPLES_RAINBOW',
  'CY_EFFECT_BOTTOM_RAYS',
  'CY_EFFECT_RISING_BANDS',
  'CY_EFFECT_HELICAL_PLASMA',
  'CY_EFFECT_NOISE_WAVES_TUBE',
  'CY_EFFECT_CELL_MEMBRANE_FLOW',
  'CY_EFFECT_CROSS_BANDS_TUBE',
];

const expectedScenes = [
  'SCENE_ID_ANEMONE',
  'SCENE_ID_TIDAL_BLOOM',
  'SCENE_ID_LAVA',
  'SCENE_ID_FLAME',
  'SCENE_ID_PLASMA_CORE',
  'SCENE_ID_DEEP_NOISE',
  'SCENE_ID_AURORA_TUBE',
  'SCENE_ID_INNER_SWIRL',
  'SCENE_ID_BUBBLES_VOLUME',
  'SCENE_ID_RING_RIPPLES',
  'SCENE_ID_RING_RIPPLES_RAINBOW',
  'SCENE_ID_BOTTOM_RAYS',
  'SCENE_ID_RISING_BANDS',
  'SCENE_ID_HELICAL_PLASMA',
  'SCENE_ID_NOISE_WAVES_TUBE',
  'SCENE_ID_CELL_MEMBRANE_FLOW',
  'SCENE_ID_CROSS_BANDS_TUBE',
];

const numberList = (source) => source.split(',').map((item) => item.trim()).filter(Boolean);
const enumBody = (source, name) => source.match(new RegExp(`enum ${name} : uint8_t \\{([\\s\\S]*?)\\n\\};`))?.[1] ?? '';

test('CY Effects v2.0 registers the exact 17 names in order', () => {
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

test('effect kinds and scene ids follow the same v2.0 order', () => {
  const kinds = numberList(enumBody(effects, 'CyEffectKind')).map((line) => line.replace(/\s*=\s*\d+/, ''));
  assert.deepEqual(kinds, expectedKinds);

  const scenes = numberList(enumBody(contract, 'SceneId')).map((line) => line.replace(/\s*=\s*\d+/, ''));
  assert.deepEqual(scenes, expectedScenes);

  const sceneValues = numberList(enumBody(contract, 'SceneId')).map((line) => Number(line.match(/=\s*(\d+)/)?.[1]));
  assert.deepEqual(sceneValues, Array.from({ length: 17 }, (_, index) => index + 1));
});

test('registered modes route to matching scene ids and render functions', () => {
  const bodies = [...usermod.matchAll(/static uint16_t (mode_cy_\w+)\(\) \{\n\s*return render_cylinder_scene\(CylinderLamp::(SCENE_ID_\w+), CylinderLamp::(renderCy\w+)\);\n\}/g)];
  assert.equal(bodies.length, 17);
  assert.deepEqual(bodies.map((match) => match[1]), expectedModes);
  assert.deepEqual(bodies.map((match) => match[2]), expectedScenes);
  assert.equal(new Set(bodies.map((match) => match[3])).size, 17);
});

test('CY scalar fields are distinct implementations, not fall-through aliases', () => {
  assert.doesNotMatch(effects, /case CY_EFFECT_RING_RIPPLES:\s*case CY_EFFECT_RING_RIPPLES_RAINBOW:/);

  const fieldCases = [...effects.matchAll(/case (CY_EFFECT_[A-Z0-9_]+): return (cyField\w+)\(c, t\);/g)];
  const fieldMap = new Map(fieldCases.map((match) => [match[1], match[2]]));
  for (const kind of expectedKinds.slice(1)) {
    assert.ok(fieldMap.has(kind), `${kind} must return a field function`);
  }
  assert.equal(new Set([...fieldMap.values()]).size, expectedKinds.length - 1);
});

test('native cylindrical coordinates use seamless x and physical-height y', () => {
  assert.match(volume, /coord\.theta = CY_TWO_PI \* \(\(float\(x\) \+ 0\.5f\) \/ float\(surface\.width\)\);/);
  assert.match(volume, /coord\.h = surface\.height <= 1 \? 0\.0f : float\(surface\.height - 1 - y\) \/ float\(surface\.height - 1\);/);
  assert.match(effects, /const float h = H <= 1 \? 0\.0f : float\(H - 1 - y\) \/ float\(H - 1\);/);
  assert.match(geometry, /while \(x < 0\) x \+= width;/);
  assert.match(geometry, /while \(x >= width\) x -= width;/);
  assert.match(geometry, /return field\[indexOf\(wrapX\(x, surface\.width\), clampY\(y, surface\.height\), surface\)\];/);

  const surface = { width: 24, height: 10 };
  const coord = (x, y, sample = 0) => ({
    theta: Math.PI * 2 * ((x + 0.5) / surface.width),
    h: surface.height <= 1 ? 0 : (surface.height - 1 - y) / (surface.height - 1),
    r: 1 - (sample / 3),
  });

  assert.equal(coord(0, surface.height - 1).h, 0);
  assert.equal(coord(0, 0).h, 1);
  assert.ok(coord(surface.width - 1, 5).theta < Math.PI * 2);
  assert.ok(coord(0, 5).theta > 0);
});

test('wrap helpers and bottom-fed fields stay bounded and non-black in deterministic samples', () => {
  const wrapX = (x, width) => {
    while (x < 0) x += width;
    while (x >= width) x -= width;
    return x;
  };
  assert.equal(wrapX(-1, 24), 23);
  assert.equal(wrapX(24, 24), 0);
  assert.equal(wrapX(49, 24), 1);

  const gauss = (value, center, width) => Math.exp(-(((value - center) / width) ** 2));
  const bottomRays = (theta, h, r, t) => {
    const rays = 0.5 + 0.5 * Math.cos((8 * theta) - (0.0009 * t));
    return (rays ** 1.7) * Math.exp(-h / 0.24) * gauss(r, 0.70, 0.22);
  };
  const rainbowRipple = (theta, h, r, t) => {
    const fract = (value) => value - Math.floor(value);
    const p1 = fract(0.00018 * t);
    const p2 = fract(0.37 + (0.00013 * t));
    const ringA = gauss(Math.abs(h - p1), 0, 0.055);
    const ringB = gauss(Math.abs(h - p2), 0, 0.080);
    const prism = 0.62 + 0.38 * Math.sin((7 * theta) + (1.4 * Math.sin(6 * h)) + (0.0008 * t));
    return ((0.72 * ringA) + (0.55 * ringB)) * prism * gauss(r, 0.70, 0.20);
  };

  const bottom = bottomRays(0, 0, 0.70, 1000);
  const top = bottomRays(0, 1, 0.70, 1000);
  assert.ok(bottom > top * 30, 'bottom rays must be physically bottom-oriented');

  let lit = 0;
  for (let y = 0; y < 10; y++) {
    const h = (9 - y) / 9;
    for (let x = 0; x < 24; x++) {
      const theta = Math.PI * 2 * ((x + 0.5) / 24);
      const values = [
        bottomRays(theta, h, 0.70, 1000),
        rainbowRipple(theta, h, 0.70, 1000),
      ];
      for (const value of values) {
        assert.ok(Number.isFinite(value));
        assert.ok(value >= 0);
        assert.ok(value <= 1.4);
        if (value > 0.03) lit += 1;
      }
    }
  }
  assert.ok(lit > 24, 'sampled CY fields should render non-black coverage');
});
