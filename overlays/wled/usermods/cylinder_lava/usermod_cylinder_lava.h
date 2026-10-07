#pragma once

#include "wled.h"
#include "FX.h"
#include "cylinder_lava_engine.h"

typedef void (*CylinderRenderFn)(CylinderLamp::RenderState& state, const CylinderLamp::Surface& surface, uint16_t dt);

struct CylinderRuntimeState {
  CylinderLamp::RenderState render;
};

static uint16_t render_cylinder_scene(uint8_t sceneId, CylinderRenderFn renderFn);

#define CY_DECLARE_MODE(NAME) static uint16_t mode_cy_##NAME();
CY_DECLARE_MODE(tidal_bloom)
CY_DECLARE_MODE(flame)
CY_DECLARE_MODE(lava_flow)
CY_DECLARE_MODE(ocean_drift)
CY_DECLARE_MODE(aurora)
CY_DECLARE_MODE(nebula)
CY_DECLARE_MODE(energy_pulse)
CY_DECLARE_MODE(bubbles)
CY_DECLARE_MODE(particle_storm)
CY_DECLARE_MODE(rainbow_spiral)
CY_DECLARE_MODE(matrix_rain)
CY_DECLARE_MODE(electric_storm)
CY_DECLARE_MODE(heartbeat)
CY_DECLARE_MODE(dreamscape)
CY_DECLARE_MODE(anemone)
CY_DECLARE_MODE(ripple_rings)
CY_DECLARE_MODE(comet_trails)
#undef CY_DECLARE_MODE

static const char _data_FX_MODE_CY_TIDAL_BLOOM[] PROGMEM =
  "CY Tidal Bloom@Flow,Scale,Energy,Stability,Softness;Deep,Bloom,Tip;!;02;m12=0,sx=92,ix=132,c1=166,c2=148,c3=112";
static const char _data_FX_MODE_CY_FLAME[] PROGMEM =
  "CY Flame@Flow,Scale,Energy,Stability,Softness;Ember,Flame,Core;!;02;m12=0,sx=150,ix=130,c1=206,c2=112,c3=60";
static const char _data_FX_MODE_CY_LAVA_FLOW[] PROGMEM =
  "CY Lava Flow@Flow,Scale,Energy,Stability,Softness;Liquid,Lava,Core;!;02;m12=0,sx=58,ix=150,c1=170,c2=190,c3=120";
static const char _data_FX_MODE_CY_OCEAN_DRIFT[] PROGMEM =
  "CY Ocean Drift@Flow,Scale,Energy,Stability,Softness;Deep,Wave,Foam;!;02;m12=0,sx=78,ix=122,c1=154,c2=172,c3=132";
static const char _data_FX_MODE_CY_AURORA[] PROGMEM =
  "CY Aurora@Flow,Scale,Energy,Stability,Softness;Night,Glow,Crown;!;02;m12=0,sx=88,ix=128,c1=176,c2=150,c3=120";
static const char _data_FX_MODE_CY_NEBULA[] PROGMEM =
  "CY Nebula@Flow,Scale,Energy,Stability,Softness;Void,Cloud,Star;!;02;m12=0,sx=76,ix=142,c1=180,c2=154,c3=126";
static const char _data_FX_MODE_CY_ENERGY_PULSE[] PROGMEM =
  "CY Energy Pulse@Flow,Scale,Energy,Stability,Softness;Void,Pulse,Core;!;02;m12=0,sx=124,ix=140,c1=196,c2=116,c3=76";
static const char _data_FX_MODE_CY_BUBBLES[] PROGMEM =
  "CY Bubbles@Flow,Scale,Energy,Stability,Softness;Depth,Bubble,Highlight;!;02;m12=0,sx=72,ix=128,c1=178,c2=160,c3=128";
static const char _data_FX_MODE_CY_PARTICLE_STORM[] PROGMEM =
  "CY Particle Storm@Flow,Scale,Energy,Stability,Softness;Void,Particle,Spark;!;02;m12=0,sx=128,ix=150,c1=188,c2=132,c3=90";
static const char _data_FX_MODE_CY_RAINBOW_SPIRAL[] PROGMEM =
  "CY Rainbow Spiral@Flow,Scale,Energy,Stability,Softness;!;!;02;m12=0,sx=112,ix=128,c1=190,c2=124,c3=90";
static const char _data_FX_MODE_CY_MATRIX_RAIN[] PROGMEM =
  "CY Matrix Rain@Flow,Scale,Energy,Stability,Softness;Dark,Trail,Head;!;02;m12=0,sx=108,ix=128,c1=178,c2=136,c3=92";
static const char _data_FX_MODE_CY_ELECTRIC_STORM[] PROGMEM =
  "CY Electric Storm@Flow,Scale,Energy,Stability,Softness;Void,Arc,Flash;!;02;m12=0,sx=138,ix=140,c1=188,c2=126,c3=84";
static const char _data_FX_MODE_CY_HEARTBEAT[] PROGMEM =
  "CY Heartbeat@Flow,Scale,Energy,Stability,Softness;Dark,Pulse,Peak;!;02;m12=0,sx=92,ix=128,c1=176,c2=142,c3=104";
static const char _data_FX_MODE_CY_DREAMSCAPE[] PROGMEM =
  "CY Dreamscape@Flow,Scale,Energy,Stability,Softness;Night,Dream,Glow;!;02;m12=0,sx=84,ix=128,c1=172,c2=150,c3=120";
static const char _data_FX_MODE_CY_ANEMONE[] PROGMEM =
  "CY Anemone@Flow,Scale,Energy,Stability,Softness;Deep,Organism,Tip;!;02;m12=0,sx=96,ix=120,c1=164,c2=152,c3=108";
static const char _data_FX_MODE_CY_RIPPLE_RINGS[] PROGMEM =
  "CY Ripple Rings@Flow,Scale,Energy,Stability,Softness;Depth,Ripple,Highlight;!;02;m12=0,sx=100,ix=128,c1=180,c2=130,c3=96";
static const char _data_FX_MODE_CY_COMET_TRAILS[] PROGMEM =
  "CY Comet Trails@Flow,Scale,Energy,Stability,Softness;Void,Trail,Head;!;02;m12=0,sx=118,ix=138,c1=184,c2=132,c3=96";

static uint16_t render_cylinder_scene(uint8_t sceneId, CylinderRenderFn renderFn) {
  if (!SEGENV.allocateData(sizeof(CylinderRuntimeState))) {
    CylinderLamp::Native2DCylinderAdapter::fill(CylinderLamp::Native2DCylinderAdapter::color(0));
    return FRAMETIME;
  }

  CylinderRuntimeState* runtime = reinterpret_cast<CylinderRuntimeState*>(SEGENV.data);
  CylinderLamp::RenderState* state = &runtime->render;
  CylinderLamp::Surface surface;
  if (!CylinderLamp::prepare(*state, surface)) {
    CylinderLamp::Native2DCylinderAdapter::fill(CylinderLamp::Native2DCylinderAdapter::color(0));
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

#define CY_MODE(NAME, SCENE, RENDER) \
  static uint16_t mode_cy_##NAME() { \
    return render_cylinder_scene(CylinderLamp::SCENE, CylinderLamp::RENDER); \
  }

CY_MODE(tidal_bloom, SCENE_ID_TIDAL_BLOOM, renderCyTidalBloom)
CY_MODE(flame, SCENE_ID_FLAME, renderCyFlame)
CY_MODE(lava_flow, SCENE_ID_LAVA_FLOW, renderCyLavaFlow)
CY_MODE(ocean_drift, SCENE_ID_OCEAN_DRIFT, renderCyOceanDrift)
CY_MODE(aurora, SCENE_ID_AURORA, renderCyAurora)
CY_MODE(nebula, SCENE_ID_NEBULA, renderCyNebula)
CY_MODE(energy_pulse, SCENE_ID_ENERGY_PULSE, renderCyEnergyPulse)
CY_MODE(bubbles, SCENE_ID_BUBBLES, renderCyBubbles)
CY_MODE(particle_storm, SCENE_ID_PARTICLE_STORM, renderCyParticleStorm)
CY_MODE(rainbow_spiral, SCENE_ID_RAINBOW_SPIRAL, renderCyRainbowSpiral)
CY_MODE(matrix_rain, SCENE_ID_MATRIX_RAIN, renderCyMatrixRain)
CY_MODE(electric_storm, SCENE_ID_ELECTRIC_STORM, renderCyElectricStorm)
CY_MODE(heartbeat, SCENE_ID_HEARTBEAT, renderCyHeartbeat)
CY_MODE(dreamscape, SCENE_ID_DREAMSCAPE, renderCyDreamscape)
CY_MODE(anemone, SCENE_ID_ANEMONE, renderCyAnemone)
CY_MODE(ripple_rings, SCENE_ID_RIPPLE_RINGS, renderCyRippleRings)
CY_MODE(comet_trails, SCENE_ID_COMET_TRAILS, renderCyCometTrails)
#undef CY_MODE

class CylinderLavaUsermod : public Usermod {
private:
  bool initDone = false;

public:
  void setup() override {
    if (initDone) return;
    strip.addEffect(255, &mode_cy_tidal_bloom, _data_FX_MODE_CY_TIDAL_BLOOM);
    strip.addEffect(255, &mode_cy_flame, _data_FX_MODE_CY_FLAME);
    strip.addEffect(255, &mode_cy_lava_flow, _data_FX_MODE_CY_LAVA_FLOW);
    strip.addEffect(255, &mode_cy_ocean_drift, _data_FX_MODE_CY_OCEAN_DRIFT);
    strip.addEffect(255, &mode_cy_aurora, _data_FX_MODE_CY_AURORA);
    strip.addEffect(255, &mode_cy_nebula, _data_FX_MODE_CY_NEBULA);
    strip.addEffect(255, &mode_cy_energy_pulse, _data_FX_MODE_CY_ENERGY_PULSE);
    strip.addEffect(255, &mode_cy_bubbles, _data_FX_MODE_CY_BUBBLES);
    strip.addEffect(255, &mode_cy_particle_storm, _data_FX_MODE_CY_PARTICLE_STORM);
    strip.addEffect(255, &mode_cy_rainbow_spiral, _data_FX_MODE_CY_RAINBOW_SPIRAL);
    strip.addEffect(255, &mode_cy_matrix_rain, _data_FX_MODE_CY_MATRIX_RAIN);
    strip.addEffect(255, &mode_cy_electric_storm, _data_FX_MODE_CY_ELECTRIC_STORM);
    strip.addEffect(255, &mode_cy_heartbeat, _data_FX_MODE_CY_HEARTBEAT);
    strip.addEffect(255, &mode_cy_dreamscape, _data_FX_MODE_CY_DREAMSCAPE);
    strip.addEffect(255, &mode_cy_anemone, _data_FX_MODE_CY_ANEMONE);
    strip.addEffect(255, &mode_cy_ripple_rings, _data_FX_MODE_CY_RIPPLE_RINGS);
    strip.addEffect(255, &mode_cy_comet_trails, _data_FX_MODE_CY_COMET_TRAILS);
    initDone = true;
  }

  void loop() override {
  }
};

#ifdef REGISTER_USERMOD
static CylinderLavaUsermod cylinder_lava_usermod;
REGISTER_USERMOD(cylinder_lava_usermod);
#endif
