#pragma once

#include "wled.h"
#include "FX.h"
#include "cylinder_lava_engine.h"
#include "native_2d_cylinder_adapter.h"

typedef void (*CylinderRenderFn)(CylinderLamp::RenderState& state, const CylinderLamp::Surface& surface, uint16_t dt);

struct CylinderRuntimeState {
  CylinderLamp::RenderState render;
};

static void render_cylinder_scene(uint8_t sceneId, CylinderRenderFn renderFn);
static void mode_cy_anemone();
static void mode_cy_tidal_bloom();
static void mode_cy_lava_lamp();
static void mode_cy_flame();
static void mode_cy_plasma_core();
static void mode_cy_deep_noise();
static void mode_cy_aurora_tube();
static void mode_cy_inner_swirl();
static void mode_cy_bubbles_volume();
static void mode_cy_ring_ripples();
static void mode_cy_ring_ripples_rainbow();
static void mode_cy_bottom_rays();
static void mode_cy_rising_bands();
static void mode_cy_helical_plasma();
static void mode_cy_noise_waves_tube();
static void mode_cy_cell_membrane_flow();
static void mode_cy_cross_bands_tube();

static const char _data_FX_MODE_CY_ANEMONE[] PROGMEM =
  "CY Anemone@Flow,X Scale,Y Scale,Stability,Legs;Liquid,Organism,Tip;!;02;m12=0,sx=96,ix=51,c1=51,c2=160,c3=108";

static const char _data_FX_MODE_CY_TIDAL_BLOOM[] PROGMEM =
  "CY Tidal Bloom@Flow,X Scale,Y Scale,Variation,Legs;Liquid,Bloom,Tip;!;02;m12=0,sx=96,ix=51,c1=51,c2=160,c3=108";

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

static void render_cylinder_scene(uint8_t sceneId, CylinderRenderFn renderFn) {
  if (!SEGENV.allocateData(sizeof(CylinderRuntimeState))) {
    SEGMENT.fill(SEGCOLOR(0));
    return;
  }

  CylinderRuntimeState* runtime = reinterpret_cast<CylinderRuntimeState*>(SEGENV.data);
  CylinderLamp::RenderState* state = &runtime->render;
  CylinderLamp::Surface surface;
  if (!CylinderLamp::prepare(*state, surface)) {
    SEGMENT.fill(SEGCOLOR(0));
    return;
  }

#ifdef CYLINDER_DEBUG_PATTERN
  CylinderLamp::renderDebugPattern(surface);
  return;
#endif

  CylinderLamp::selectScene(*state, sceneId);

  const uint16_t dt = CylinderLamp::elapsedMs(*state);
  renderFn(*state, surface, dt);
  return;
}

static void mode_cy_anemone() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_ANEMONE, CylinderLamp::renderCyAnemone);
}

static void mode_cy_tidal_bloom() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_TIDAL_BLOOM, CylinderLamp::renderCyTidalBloom);
}

static void mode_cy_lava_lamp() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_LAVA, CylinderLamp::renderCyLavaLamp);
}

static void mode_cy_flame() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_FLAME, CylinderLamp::renderCyFlame);
}

static void mode_cy_plasma_core() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_PLASMA_CORE, CylinderLamp::renderCyPlasmaCore);
}

static void mode_cy_deep_noise() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_DEEP_NOISE, CylinderLamp::renderCyDeepNoise);
}

static void mode_cy_aurora_tube() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_AURORA_TUBE, CylinderLamp::renderCyAuroraTube);
}

static void mode_cy_inner_swirl() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_INNER_SWIRL, CylinderLamp::renderCyInnerSwirl);
}

static void mode_cy_bubbles_volume() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_BUBBLES_VOLUME, CylinderLamp::renderCyBubblesVolume);
}

static void mode_cy_ring_ripples() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_RING_RIPPLES, CylinderLamp::renderCyRingRipples);
}

static void mode_cy_ring_ripples_rainbow() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_RING_RIPPLES_RAINBOW, CylinderLamp::renderCyRingRipplesRainbow);
}

static void mode_cy_bottom_rays() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_BOTTOM_RAYS, CylinderLamp::renderCyBottomRays);
}

static void mode_cy_rising_bands() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_RISING_BANDS, CylinderLamp::renderCyRisingBands);
}

static void mode_cy_helical_plasma() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_HELICAL_PLASMA, CylinderLamp::renderCyHelicalPlasma);
}

static void mode_cy_noise_waves_tube() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_NOISE_WAVES_TUBE, CylinderLamp::renderCyNoiseWavesTube);
}

static void mode_cy_cell_membrane_flow() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_CELL_MEMBRANE_FLOW, CylinderLamp::renderCyCellMembraneFlow);
}

static void mode_cy_cross_bands_tube() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_CROSS_BANDS_TUBE, CylinderLamp::renderCyCrossBandsTube);
}

class CylinderLavaUsermod : public Usermod {
private:
  bool initDone = false;

public:
  void setup() override {
    if (initDone) return;
    CylinderNative::registerCyIdentity(0, strip.addEffect(255, &mode_cy_anemone, _data_FX_MODE_CY_ANEMONE));
    CylinderNative::registerCyIdentity(1, strip.addEffect(255, &mode_cy_tidal_bloom, _data_FX_MODE_CY_TIDAL_BLOOM));
    CylinderNative::registerCyIdentity(2, strip.addEffect(255, &mode_cy_lava_lamp, _data_FX_MODE_CY_LAVA_LAMP));
    CylinderNative::registerCyIdentity(3, strip.addEffect(255, &mode_cy_flame, _data_FX_MODE_CY_FLAME));
    CylinderNative::registerCyIdentity(4, strip.addEffect(255, &mode_cy_plasma_core, _data_FX_MODE_CY_PLASMA_CORE));
    CylinderNative::registerCyIdentity(5, strip.addEffect(255, &mode_cy_deep_noise, _data_FX_MODE_CY_DEEP_NOISE));
    CylinderNative::registerCyIdentity(6, strip.addEffect(255, &mode_cy_aurora_tube, _data_FX_MODE_CY_AURORA_TUBE));
    CylinderNative::registerCyIdentity(7, strip.addEffect(255, &mode_cy_inner_swirl, _data_FX_MODE_CY_INNER_SWIRL));
    CylinderNative::registerCyIdentity(8, strip.addEffect(255, &mode_cy_bubbles_volume, _data_FX_MODE_CY_BUBBLES_VOLUME));
    CylinderNative::registerCyIdentity(9, strip.addEffect(255, &mode_cy_ring_ripples, _data_FX_MODE_CY_RING_RIPPLES));
    CylinderNative::registerCyIdentity(10, strip.addEffect(255, &mode_cy_ring_ripples_rainbow, _data_FX_MODE_CY_RING_RIPPLES_RAINBOW));
    CylinderNative::registerCyIdentity(11, strip.addEffect(255, &mode_cy_bottom_rays, _data_FX_MODE_CY_BOTTOM_RAYS));
    CylinderNative::registerCyIdentity(12, strip.addEffect(255, &mode_cy_rising_bands, _data_FX_MODE_CY_RISING_BANDS));
    CylinderNative::registerCyIdentity(13, strip.addEffect(255, &mode_cy_helical_plasma, _data_FX_MODE_CY_HELICAL_PLASMA));
    CylinderNative::registerCyIdentity(14, strip.addEffect(255, &mode_cy_noise_waves_tube, _data_FX_MODE_CY_NOISE_WAVES_TUBE));
    CylinderNative::registerCyIdentity(15, strip.addEffect(255, &mode_cy_cell_membrane_flow, _data_FX_MODE_CY_CELL_MEMBRANE_FLOW));
    CylinderNative::registerCyIdentity(16, strip.addEffect(255, &mode_cy_cross_bands_tube, _data_FX_MODE_CY_CROSS_BANDS_TUBE));
    initDone = true;
  }

  void loop() override {
  }

  // Read-only manifest for offline preset generation. Never edits stored presets.
  void addToJsonInfo(JsonObject& info) override {
    JsonObject identities = info.createNestedObject("cyEffectIds");
    for (const char* name : CylinderNative::cyNames) {
      const int id = CylinderNative::resolveCyIdentity(name);
      if (id >= 0) identities[name] = id;
    }
  }
};

#ifdef USERMOD_CYLINDER_LAVA
static CylinderLavaUsermod cylinderLavaUsermod;
REGISTER_USERMOD(cylinderLavaUsermod);
#endif
