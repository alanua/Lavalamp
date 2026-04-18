#pragma once

#include "wled.h"
#include "FX.h"
#include "cylinder_lava_engine.h"

static uint16_t mode_cylinder_lava();
static uint16_t mode_cylinder_flame();

static const char _data_FX_MODE_CYLINDER_LAVA[] PROGMEM =
  "Lava Lamp@Flow,Blob size,Bottom heat,Viscosity,Softness;Deep,Glow,Core;!;2;sx=56,ix=178,c1=136,c2=214,c3=150";

static const char _data_FX_MODE_CYLINDER_FLAME[] PROGMEM =
  "Flame@Rise,Width,Base heat,Stability,Softness;Ember,Flame,Core;!;2;sx=132,ix=144,c1=202,c2=132,c3=64";

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

class CylinderLavaUsermod : public Usermod {
private:
  bool initDone = false;

public:
  void setup() override {
    if (initDone) return;
    strip.addEffect(255, &mode_cylinder_lava, _data_FX_MODE_CYLINDER_LAVA);
    strip.addEffect(255, &mode_cylinder_flame, _data_FX_MODE_CYLINDER_FLAME);
    initDone = true;
  }

  void loop() override {
  }
};
