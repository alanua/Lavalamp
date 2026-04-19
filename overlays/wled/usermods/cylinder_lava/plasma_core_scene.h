#pragma once

#include "cylinder_volume.h"

namespace CylinderLamp {

static MotionRates plasmaCoreMotionRates(const SceneControls& controls) {
  MotionRates rates;
  rates.angular = 8 + scale8(controls.flow, 14);
  rates.rise = 7 + scale8(controls.flow, 12);
  rates.rotation = 9 + scale8(controls.flow, 14);
  rates.deform = 7 + scale8(controls.softness, 12);
  return rates;
}

static PipelineSettings plasmaCorePipelineSettings(const SceneContext& context) {
  PipelineSettings settings;
  settings.temporalAlpha = 88 + scale8(context.controls.viscosity, 62);
  settings.blurX = 6 + scale8(context.controls.softness, 16);
  settings.blurY = 6 + scale8(context.controls.softness, 14);
  settings.floor = 10;
  settings.ceiling = 232;
  settings.contrast = 174;
  return settings;
}

static inline uint8_t plasmaCoreSample(uint8_t h, uint8_t angle, uint8_t sample, const SceneContext& context) {
  const uint8_t depth = cylinderDepth8(sample);
  const uint8_t radius = cylinderRadius8(depth);

  const uint8_t core = bellCurve8(radius, 71, 46);
  const uint8_t vmod = 184 + scale8(sin8_t(scale8(h, 244) - phase8(context.motion.risePhase)), 71);

  const uint8_t arc1 = sin8_t(uint8_t(angle * 3U + scale8(h, 183) - phase8(context.motion.rotationPhase)));
  const uint8_t arc2 = sin8_t(uint8_t(angle * 5U - scale8(h, 130) + phase8(context.motion.deformPhase)));
  const uint8_t arcs = uint8_t((uint16_t(arc1) * 141U + uint16_t(arc2) * 115U) >> 8);
  const uint8_t shapedArcs = scale8(ease8InOutApprox(arcs), arcs);

  const uint16_t plasmaScale = 147 - scale8(radius, 58);
  const uint8_t plasma = wrappedCylinderNoiseFixed8(angle, h, plasmaScale, 256, int32_t(context.motion.anglePhase) >> 4, context.motion.rotationPhase >> 4);
  const uint8_t filamentBoost = qadd8(166, scale8(plasma, 178));
  const uint8_t filament = scale8(shapedArcs, filamentBoost);
  const uint8_t shellReach = bellCurve8(radius, 199, 41);

  uint8_t field = 0;
  addLayer(field, scale8(core, vmod), 178);
  addLayer(field, scale8(filament, shellReach), 242);
  return field;
}

static void buildPlasmaCoreField(SceneContext& context) {
  advanceMotion(context.motion, context.dt, plasmaCoreMotionRates(context.controls));

  const uint8_t flickerA = scale8(sin8_t(phase8(context.motion.rotationPhase)), 26);
  const uint8_t flickerB = scale8(sin8_t(phase8(context.motion.deformPhase)), 10);
  const uint8_t flicker = qadd8(230, qadd8(flickerA, flickerB));
  const uint8_t heatGain = 190 + scale8(context.controls.heat, 48);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    const uint8_t h = cylinderHeight8(y, context.surface);

    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t angle = angle8(x, context.surface.width);
      uint32_t accumulated = 0;

      for (uint8_t sample = 0; sample < CYLINDER_DEPTH_SAMPLES; sample++) {
        addWeightedDepth(accumulated, plasmaCoreSample(h, angle, sample, context), sample, DEPTH_FADE_PLASMA);
      }

      uint8_t energy = normalizeDepthEnergy(accumulated, DEPTH_FADE_PLASMA);
      energy = scale8(scale8(energy, flicker), heatGain);
      context.field.raw[indexOf(x, y, context.surface)] = energy;
    }
  }
}

static inline CRGB plasmaCorePalette(uint8_t value, const CRGB& low, const CRGB& mid, const CRGB& high) {
  if (value < 74) return blendRgb(CRGB::Black, low, uint8_t((uint16_t(value) * 255U) / 74U));
  if (value < 154) return blendRgb(low, mid, uint8_t((uint16_t(value - 74) * 255U) / 80U));
  if (value < 222) return blendRgb(mid, high, uint8_t((uint16_t(value - 154) * 255U) / 68U));
  return blendRgb(high, CRGB(210, 248, 255), uint8_t((uint16_t(value - 222) * 255U) / 33U));
}

static void outputPlasmaCore(SceneContext& context) {
  const CRGB low = colorOr(0, CRGB(8, 0, 46));
  const CRGB mid = colorOr(1, CRGB(0, 138, 230));
  const CRGB high = colorOr(2, CRGB(186, 58, 238));

  for (uint8_t y = 0; y < context.surface.height; y++) {
    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t scalar = context.field.blurred[indexOf(x, y, context.surface)];
      CRGB color = plasmaCorePalette(ease8InOutApprox(scalar), low, mid, high);
      color.nscale8_video(qadd8(8, scale8(scalar, 222)));
      limitColor(color, 230, 245, 255, 500);
      SEGMENT.setPixelColorXY(x, y, color);
    }
  }
}

static const SceneDefinition PLASMA_CORE_SCENE = {
  buildPlasmaCoreField,
  plasmaCorePipelineSettings,
  outputPlasmaCore
};

static void renderPlasmaCore(RenderState& state, const Surface& surface, uint16_t dt) {
  renderScene(state, surface, dt, PLASMA_CORE_SCENE);
}

} // namespace CylinderLamp
