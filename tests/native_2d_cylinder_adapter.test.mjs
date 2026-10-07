import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import test from 'node:test';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const dir = path.join(root, 'overlays/wled/usermods/cylinder_lava');
const adapter = fs.readFileSync(path.join(dir, 'native_2d_cylinder_adapter.h'), 'utf8');
const contract = fs.readFileSync(path.join(dir, 'cylinder_scene_contract.h'), 'utf8');
const usermod = fs.readFileSync(path.join(dir, 'usermod_cylinder_lava.h'), 'utf8');
const cyEffects = fs.readFileSync(path.join(dir, 'cy_effects_scene.h'), 'utf8');
const lava = fs.readFileSync(path.join(dir, 'lava_scene.h'), 'utf8');
const flame = fs.readFileSync(path.join(dir, 'flame_scene.h'), 'utf8');
const pipeline = fs.readFileSync(path.join(dir, 'cylinder_pipeline.h'), 'utf8');
const platformio = fs.readFileSync(path.join(root, 'overlays/wled/platformio_override.ini'), 'utf8');

test('native 2D adapter centralizes WLED matrix access', () => {
  assert.match(adapter, /struct Native2DCylinderAdapter/);
  assert.match(adapter, /strip\.isMatrix && SEGMENT\.is2D\(\)/);
  assert.match(adapter, /SEGMENT\.virtualWidth\(\)/);
  assert.match(adapter, /SEGMENT\.virtualHeight\(\)/);
  assert.match(adapter, /strip\.now/);
  assert.match(adapter, /SEGMENT\.setPixelColorXY\(x, y, color\)/);
  assert.match(adapter, /SEGMENT\.fill\(color\)/);
});

test('scene preparation and output use the adapter instead of direct native 2D calls', () => {
  assert.match(contract, /Native2DCylinderAdapter::available\(\)/);
  assert.match(contract, /Native2DCylinderAdapter::width\(\)/);
  assert.match(contract, /Native2DCylinderAdapter::height\(\)/);
  assert.match(contract, /Native2DCylinderAdapter::now\(\)/);

  for (const source of [cyEffects, lava, flame]) {
    assert.match(source, /Native2DCylinderAdapter::setPixel\(x, y, color\)/);
    assert.doesNotMatch(source, /SEGMENT\.setPixelColorXY/);
  }

  assert.match(pipeline, /Native2DCylinderAdapter::color\(slot\)/);
  assert.match(usermod, /Native2DCylinderAdapter::fill\(CylinderLamp::Native2DCylinderAdapter::color\(0\)\)/);
});

test('WLED 16 integration self-registers and enables the cylinder usermod', () => {
  assert.match(usermod, /#ifdef REGISTER_USERMOD/);
  assert.match(usermod, /REGISTER_USERMOD\(cylinder_lava_usermod\)/);
  assert.match(platformio, /custom_usermods =\n\s+cylinder_lava/);
});
