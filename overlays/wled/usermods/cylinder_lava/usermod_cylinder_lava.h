#pragma once

#include "wled.h"
#include "FX.h"
#include "cylinder_lava_engine.h"

static uint16_t mode_cylinder_lava();
static uint16_t mode_cylinder_flame();
static uint16_t mode_cylinder_anemone();
static uint16_t mode_cylinder_plasma_core();
static uint16_t mode_cylinder_deep_noise();

static const char _data_FX_MODE_CYLINDER_LAVA[] PROGMEM =
  "Lava Lamp@Flow,Blob size,Bottom heat,Viscosity,Softness;Deep,Glow,Core;!;2;sx=56,ix=178,c1=136,c2=214,c3=150";

static const char _data_FX_MODE_CYLINDER_FLAME[] PROGMEM =
  "Flame@Rise,Width,Base heat,Stability,Softness;Ember,Flame,Core;!;2;sx=136,ix=122,c1=204,c2=124,c3=68";

static const char _data_FX_MODE_CYLINDER_ANEMONE[] PROGMEM =
  "Anemone@Flow,Body size,Energy,Stability,Softness;Deep,Petal,Tip;!;2;sx=96,ix=140,c1=170,c2=160,c3=108";

static const char _data_FX_MODE_CYLINDER_PLASMA_CORE[] PROGMEM =
  "Plasma Core@Flow,Core size,Energy,Stability,Softness;Void,Filament,Accent;!;2;sx=124,ix=132,c1=196,c2=116,c3=76";

static const char _data_FX_MODE_CYLINDER_DEEP_NOISE[] PROGMEM =
  "Deep Noise@Flow,Scale,Energy,Stability,Softness;Depth,Body,Highlight;!;2;sx=76,ix=150,c1=172,c2=180,c3=132";

static uint16_t mode_cylinder_lava() {
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

  CylinderLamp::selectScene(*state, CylinderLamp::SCENE_ID_LAVA);
  const uint16_t dt = CylinderLamp::elapsedMs(*state);
  CylinderLamp::renderLava(*state, surface, dt);
  return FRAMETIME;
}

static uint16_t mode_cylinder_flame() {
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

  CylinderLamp::selectScene(*state, CylinderLamp::SCENE_ID_FLAME);
  const uint16_t dt = CylinderLamp::elapsedMs(*state);
  CylinderLamp::renderFlame(*state, surface, dt);
  return FRAMETIME;
}

static uint16_t mode_cylinder_anemone() {
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

  CylinderLamp::selectScene(*state, CylinderLamp::SCENE_ID_ANEMONE);
  const uint16_t dt = CylinderLamp::elapsedMs(*state);
  CylinderLamp::renderAnemone(*state, surface, dt);
  return FRAMETIME;
}

static uint16_t mode_cylinder_plasma_core() {
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

  CylinderLamp::selectScene(*state, CylinderLamp::SCENE_ID_PLASMA_CORE);
  const uint16_t dt = CylinderLamp::elapsedMs(*state);
  CylinderLamp::renderPlasmaCore(*state, surface, dt);
  return FRAMETIME;
}

static uint16_t mode_cylinder_deep_noise() {
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

  CylinderLamp::selectScene(*state, CylinderLamp::SCENE_ID_DEEP_NOISE);
  const uint16_t dt = CylinderLamp::elapsedMs(*state);
  CylinderLamp::renderDeepNoise(*state, surface, dt);
  return FRAMETIME;
}

class CylinderLavaUsermod : public Usermod {
private:
  bool initDone = false;

public:
  void setup() override {
    if (initDone) return;
    strip.addEffect(255, &mode_cylinder_lava, _data_FX_MODE_CYLINDER_LAVA);
    strip.addEffect(255, &mode_cylinder_flame, _data_FX_MODE_CYLINDER_FLAME);
    strip.addEffect(255, &mode_cylinder_anemone, _data_FX_MODE_CYLINDER_ANEMONE);
    strip.addEffect(255, &mode_cylinder_plasma_core, _data_FX_MODE_CYLINDER_PLASMA_CORE);
    strip.addEffect(255, &mode_cylinder_deep_noise, _data_FX_MODE_CYLINDER_DEEP_NOISE);
    initDone = true;
  }

  void loop() override {
  }
};
