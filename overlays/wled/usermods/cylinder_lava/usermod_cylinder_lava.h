#pragma once

#include "wled.h"
#include "FX.h"
#include "cylinder_lava_engine.h"

static uint16_t mode_cylinder_lava();

static const char _data_FX_MODE_CYLINDER_LAVA[] PROGMEM =
  "Lava Lamp@Flow,Blob size,Bottom heat,Viscosity,Softness;Deep,Glow,Core;!;2;sx=56,ix=178,c1=136,c2=214,c3=150";

static uint16_t mode_cylinder_lava() {
  if (!SEGENV.allocateData(sizeof(CylinderLava::RenderState))) {
    SEGMENT.fill(SEGCOLOR(0));
    return FRAMETIME;
  }

  CylinderLava::RenderState* state = reinterpret_cast<CylinderLava::RenderState*>(SEGENV.data);
  CylinderLava::Surface surface;
  if (!CylinderLava::prepare(*state, surface)) {
    SEGMENT.fill(SEGCOLOR(0));
    return 350;
  }

#ifdef CYLINDER_DEBUG_PATTERN
  CylinderLava::renderDebugPattern(surface);
  return FRAMETIME;
#endif

  const uint16_t dt = CylinderLava::elapsedMs(*state);
  CylinderLava::render(*state, surface, dt);
  return FRAMETIME;
}

class CylinderLavaUsermod : public Usermod {
private:
  bool initDone = false;

public:
  void setup() override {
    if (initDone) return;
    strip.addEffect(255, &mode_cylinder_lava, _data_FX_MODE_CYLINDER_LAVA);
    initDone = true;
  }

  void loop() override {
  }
};
