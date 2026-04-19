#pragma once

#include "cylinder_volume.h"

namespace CylinderLamp {

static MotionRates deepNoiseMotionRates(const SceneControls& controls) {
  MotionRates rates;
  rates.angular = 4 + scale8(controls.flow, 7);
  rates.rise = 4 + scale8(controls.flow, 8);
  rates.rotation = 3 + scale8(controls.flow, 6);
  rates.deform = 3 + scale8(controls.softness, 5);
  return rates;
}

static PipelineSettings deepNoisePipelineSettings(const SceneContext& context) {
  PipelineSettings settings;
  settings.temporalAlpha = 132 + scale8(context.controls.viscosity, 72);
  settings.blurX = 12 + scale8(context.controls.softness, 22);
  settings.blurY = 10 + scale8(context.controls.softness, 18);
  settings.floor = 5;
  settings.ceiling = 220;
  settings.contrast = 142;
  return settings;
}

static inline uint8_t deepNoiseSample(uint8_t h, uint8_t angle, uint8_t sample, const SceneContext& context) {
  const uint8_t depth = cylinderDepth8(sample);
  const uint8_t radius = cylinderRadius8(depth);
  const uint16_t scale1 = 102 + scale8(radius, 51);
  const uint16_t scale2 = 205 + scale8(radius, 90);
  const int32_t driftPhase = -(int32_t(context.motion.risePhase) >> 4);

  const uint8_t n1 = wrappedCylinderNoiseFixed8(angle, h, scale1, 243, driftPhase);
  const uint8_t n2 = wrappedCylinderNoiseFixed8(angle + 37, h, scale2, 474, driftPhase + 57, 191);
  const uint8_t noise = uint8_t((uint16_t(n1) * 174U + uint16_t(n2) * 82U) >> 8);

  const uint8_t density = qadd8(199, scale8(bellCurve8(h, 89, 115), 56));
  const uint8_t body = bellCurve8(radius, 158, 71);
  const uint8_t drift = 224 + scale8(sin8_t(uint8_t(angle * 2U + scale8(h, 142) - phase8(context.motion.rotationPhase))), 31);

  uint8_t field = scale8(noise, density);
  field = scale8(field, body);
  field = scale8(field, drift);
  return field;
}

static void buildDeepNoiseField(SceneContext& context) {
  advanceMotion(context.motion, context.dt, deepNoiseMotionRates(context.controls));

  const uint8_t breath = 242 + scale8(sin8_t(phase8(context.motion.deformPhase)), 13);
  const uint8_t gain = 180 + scale8(context.controls.heat, 52);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    const uint8_t h = cylinderHeight8(y, context.surface);

    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t angle = angle8(x, context.surface.width);
      uint32_t accumulated = 0;

      for (uint8_t sample = 0; sample < CYLINDER_DEPTH_SAMPLES; sample++) {
        addWeightedDepth(accumulated, deepNoiseSample(h, angle, sample, context), sample, DEPTH_FADE_DEEP);
      }

      uint8_t energy = normalizeDepthEnergy(accumulated, DEPTH_FADE_DEEP);
      energy = scale8(scale8(energy, breath), gain);
      context.field.raw[indexOf(x, y, context.surface)] = energy;
    }
  }
}

static inline CRGB deepNoisePalette(uint8_t value, const CRGB& low, const CRGB& mid, const CRGB& high) {
  if (value < 88) return blendRgb(CRGB::Black, low, uint8_t((uint16_t(value) * 255U) / 88U));
  if (value < 178) return blendRgb(low, mid, uint8_t((uint16_t(value - 88) * 255U) / 90U));
  if (value < 232) return blendRgb(mid, high, uint8_t((uint16_t(value - 178) * 255U) / 54U));
  return blendRgb(high, CRGB(255, 178, 72), uint8_t((uint16_t(value - 232) * 255U) / 23U));
}

static void outputDeepNoise(SceneContext& context) {
  const CRGB low = colorOr(0, CRGB(0, 8, 34));
  const CRGB mid = colorOr(1, CRGB(0, 136, 150));
  const CRGB high = colorOr(2, CRGB(68, 224, 196));

  for (uint8_t y = 0; y < context.surface.height; y++) {
    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t scalar = context.field.blurred[indexOf(x, y, context.surface)];
      CRGB color = deepNoisePalette(ease8InOutApprox(scalar), low, mid, high);
      color.nscale8_video(qadd8(7, scale8(scalar, 210)));
      limitColor(color, 238, 230, 230, 430);
      SEGMENT.setPixelColorXY(x, y, color);
    }
  }
}

static const SceneDefinition DEEP_NOISE_SCENE = {
  buildDeepNoiseField,
  deepNoisePipelineSettings,
  outputDeepNoise
};

static void renderDeepNoise(RenderState& state, const Surface& surface, uint16_t dt) {
  renderScene(state, surface, dt, DEEP_NOISE_SCENE);
}

} // namespace CylinderLamp
