#pragma once

#include "wled.h"
#include "FX.h"
#include "cylinder_lava_engine.h"

typedef void (*CylinderRenderFn)(CylinderLamp::RenderState& state, const CylinderLamp::Surface& surface, uint16_t dt);

static uint16_t render_cylinder_scene(uint8_t sceneId, CylinderRenderFn renderFn);
static uint16_t mode_cy_anemone();
static uint16_t mode_cy_lava_lamp();
static uint16_t mode_cy_flame();
static uint16_t mode_cy_plasma_core();
static uint16_t mode_cy_deep_noise();
static uint16_t mode_cy_aurora_tube();
static uint16_t mode_cy_inner_swirl();
static uint16_t mode_cy_bubbles_volume();
static uint16_t mode_cy_ring_ripples();
static uint16_t mode_cy_ring_ripples_rainbow();
static uint16_t mode_cy_bottom_rays();
static uint16_t mode_cy_rising_bands();
static uint16_t mode_cy_helical_plasma();
static uint16_t mode_cy_noise_waves_tube();
static uint16_t mode_cy_cell_membrane_flow();
static uint16_t mode_cy_cross_bands_tube();

static const char _data_FX_MODE_CY_ANEMONE[] PROGMEM =
  "CY Anemone@Flow,Scale,Energy,Stability,Softness;Liquid,Organism,Tip;!;02;m12=0,sx=96,ix=140,c1=170,c2=160,c3=108";

static const char _data_FX_MODE_CY_LAVA_LAMP[] PROGMEM =
  "CY Lava Lamp@Flow,Scale,Energy,Stability,Softness;Liquid,Wax,Core;!;02;m12=0,sx=46,ix=150,c1=170,c2=190,c3=120";

static const char _data_FX_MODE_CY_FLAME[] PROGMEM =
  "CY Flame@Flow,Scale,Energy,Stability,Softness;Ember,Flame,Core;!;02;m12=0,sx=150,ix=130,c1=206,c2=112,c3=60";

static const char _data_FX_MODE_CY_PLASMA_CORE[] PROGMEM =
  "CY Plasma Core@Flow,Scale,Energy,Stability,Softness;Void,Filament,Core;!;02;m12=0,sx=124,ix=132,c1=196,c2=116,c3=76";

static const char _data_FX_MODE_CY_DEEP_NOISE[] PROGMEM =
  "CY Deep Noise@Flow,Scale,Energy,Stability,Softness;Depth,Body,Highlight;!;02;m12=0,sx=76,ix=150,c1=172,c2=180,c3=132";

static const char _data_FX_MODE_CY_AURORA_TUBE[] PROGMEM =
  "CY Aurora Tube@Flow,Scale,Energy,Stability,Softness;Low,Glow,Top;!;02;m12=0,sx=88,ix=128,c1=176,c2=150,c3=120";

static const char _data_FX_MODE_CY_INNER_SWIRL[] PROGMEM =
  "CY Inner Swirl@Flow,Scale,Energy,Stability,Softness;Depth,Swirl,Core;!;02;m12=0,sx=106,ix=128,c1=180,c2=140,c3=110";

static const char _data_FX_MODE_CY_BUBBLES_VOLUME[] PROGMEM =
  "CY Bubbles Volume@Flow,Scale,Energy,Stability,Softness;Depth,Bubble,Highlight;!;02;m12=0,sx=72,ix=128,c1=178,c2=160,c3=128";

static const char _data_FX_MODE_CY_RING_RIPPLES[] PROGMEM =
  "CY Ring Ripples@Flow,Scale,Energy,Stability,Softness;Depth,Ripple,Highlight;!;02;m12=0,sx=100,ix=128,c1=180,c2=130,c3=96";

static const char _data_FX_MODE_CY_RING_RIPPLES_RAINBOW[] PROGMEM =
  "CY Ring Ripples Rainbow@Flow,Scale,Energy,Stability,Softness;!;!;02;m12=0,sx=100,ix=128,c1=180,c2=130,c3=96";

static const char _data_FX_MODE_CY_BOTTOM_RAYS[] PROGMEM =
  "CY Bottom Rays@Flow,Scale,Energy,Stability,Softness;Root,Ray,Tip;!;02;m12=0,sx=104,ix=128,c1=188,c2=132,c3=84";

static const char _data_FX_MODE_CY_RISING_BANDS[] PROGMEM =
  "CY Rising Bands@Flow,Scale,Energy,Stability,Softness;!;!;02;m12=0,sx=120,ix=128,c1=180,c2=130,c3=100";

static const char _data_FX_MODE_CY_HELICAL_PLASMA[] PROGMEM =
  "CY Helical Plasma@Flow,Scale,Energy,Stability,Softness;Void,Helix,Core;!;02;m12=0,sx=112,ix=128,c1=190,c2=124,c3=90";

static const char _data_FX_MODE_CY_NOISE_WAVES_TUBE[] PROGMEM =
  "CY Noise Waves Tube@Flow,Scale,Energy,Stability,Softness;Depth,Wave,Highlight;!;02;m12=0,sx=84,ix=128,c1=172,c2=150,c3=120";

static const char _data_FX_MODE_CY_CELL_MEMBRANE_FLOW[] PROGMEM =
  "CY Cell Membrane Flow@Flow,Scale,Energy,Stability,Softness;Depth,Membrane,Highlight;!;02;m12=0,sx=76,ix=128,c1=176,c2=152,c3=124";

static const char _data_FX_MODE_CY_CROSS_BANDS_TUBE[] PROGMEM =
  "CY Cross Bands Tube@Flow,Scale,Energy,Stability,Softness;Depth,Band,Accent;!;02;m12=0,sx=104,ix=128,c1=180,c2=132,c3=96";

static uint16_t render_cylinder_scene(uint8_t sceneId, CylinderRenderFn renderFn) {
  if (!SEGENV.allocateData(sizeof(CylinderLamp::RenderState))) {
    SEGMENT.fill(SEGCOLOR(0));
    return FRAMETIME;
  }

  CylinderLamp::RenderState* state = reinterpret_cast<CylinderLamp::RenderState*>(SEGENV.data);
  CylinderLamp::Surface surface;
  if (!CylinderLamp::prepare(*state, surface)) {
    SEGMENT.fill(SEGCOLOR(0));
    return 350;
  }

#ifdef CYLINDER_DEBUG_PATTERN
  CylinderLamp::renderDebugPattern(surface);
  return FRAMETIME;
#endif

  CylinderLamp::selectScene(*state, sceneId);
  const uint16_t dt = CylinderLamp::elapsedMs(*state);
  renderFn(*state, surface, dt);
  return FRAMETIME;
}

static uint16_t mode_cy_anemone() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_ANEMONE, CylinderLamp::renderCyAnemone);
}

static uint16_t mode_cy_lava_lamp() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_LAVA, CylinderLamp::renderCyLavaLamp);
}

static uint16_t mode_cy_flame() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_FLAME, CylinderLamp::renderCyFlame);
}

static uint16_t mode_cy_plasma_core() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_PLASMA_CORE, CylinderLamp::renderCyPlasmaCore);
}

static uint16_t mode_cy_deep_noise() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_DEEP_NOISE, CylinderLamp::renderCyDeepNoise);
}

static uint16_t mode_cy_aurora_tube() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_AURORA_TUBE, CylinderLamp::renderCyAuroraTube);
}

static uint16_t mode_cy_inner_swirl() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_INNER_SWIRL, CylinderLamp::renderCyInnerSwirl);
}

static uint16_t mode_cy_bubbles_volume() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_BUBBLES_VOLUME, CylinderLamp::renderCyBubblesVolume);
}

static uint16_t mode_cy_ring_ripples() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_RING_RIPPLES, CylinderLamp::renderCyRingRipples);
}

static uint16_t mode_cy_ring_ripples_rainbow() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_RING_RIPPLES_RAINBOW, CylinderLamp::renderCyRingRipplesRainbow);
}

static uint16_t mode_cy_bottom_rays() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_BOTTOM_RAYS, CylinderLamp::renderCyBottomRays);
}

static uint16_t mode_cy_rising_bands() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_RISING_BANDS, CylinderLamp::renderCyRisingBands);
}

static uint16_t mode_cy_helical_plasma() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_HELICAL_PLASMA, CylinderLamp::renderCyHelicalPlasma);
}

static uint16_t mode_cy_noise_waves_tube() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_NOISE_WAVES_TUBE, CylinderLamp::renderCyNoiseWavesTube);
}

static uint16_t mode_cy_cell_membrane_flow() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_CELL_MEMBRANE_FLOW, CylinderLamp::renderCyCellMembraneFlow);
}

static uint16_t mode_cy_cross_bands_tube() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_CROSS_BANDS_TUBE, CylinderLamp::renderCyCrossBandsTube);
}

class CylinderLavaUsermod : public Usermod {
private:
  bool initDone = false;

public:
  void setup() override {
    if (initDone) return;
    strip.addEffect(255, &mode_cy_anemone, _data_FX_MODE_CY_ANEMONE);
    strip.addEffect(255, &mode_cy_lava_lamp, _data_FX_MODE_CY_LAVA_LAMP);
    strip.addEffect(255, &mode_cy_flame, _data_FX_MODE_CY_FLAME);
    strip.addEffect(255, &mode_cy_plasma_core, _data_FX_MODE_CY_PLASMA_CORE);
    strip.addEffect(255, &mode_cy_deep_noise, _data_FX_MODE_CY_DEEP_NOISE);
    strip.addEffect(255, &mode_cy_aurora_tube, _data_FX_MODE_CY_AURORA_TUBE);
    strip.addEffect(255, &mode_cy_inner_swirl, _data_FX_MODE_CY_INNER_SWIRL);
    strip.addEffect(255, &mode_cy_bubbles_volume, _data_FX_MODE_CY_BUBBLES_VOLUME);
    strip.addEffect(255, &mode_cy_ring_ripples, _data_FX_MODE_CY_RING_RIPPLES);
    strip.addEffect(255, &mode_cy_ring_ripples_rainbow, _data_FX_MODE_CY_RING_RIPPLES_RAINBOW);
    strip.addEffect(255, &mode_cy_bottom_rays, _data_FX_MODE_CY_BOTTOM_RAYS);
    strip.addEffect(255, &mode_cy_rising_bands, _data_FX_MODE_CY_RISING_BANDS);
    strip.addEffect(255, &mode_cy_helical_plasma, _data_FX_MODE_CY_HELICAL_PLASMA);
    strip.addEffect(255, &mode_cy_noise_waves_tube, _data_FX_MODE_CY_NOISE_WAVES_TUBE);
    strip.addEffect(255, &mode_cy_cell_membrane_flow, _data_FX_MODE_CY_CELL_MEMBRANE_FLOW);
    strip.addEffect(255, &mode_cy_cross_bands_tube, _data_FX_MODE_CY_CROSS_BANDS_TUBE);
    initDone = true;
  }

  void loop() override {
  }
};
