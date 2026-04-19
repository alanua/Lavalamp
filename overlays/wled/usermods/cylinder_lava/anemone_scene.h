#pragma once

#include "cylinder_volume.h"

namespace CylinderLamp {

static MotionRates anemoneMotionRates(const SceneControls& controls) {
  MotionRates rates;
  rates.angular = 3 + scale8(controls.flow, 4);
  rates.rise = 5 + scale8(controls.flow, 8);
  rates.rotation = 4 + scale8(controls.flow, 6);
  rates.deform = 3 + scale8(controls.softness, 5);
  return rates;
}

static PipelineSettings anemonePipelineSettings(const SceneContext& context) {
  PipelineSettings settings;
  settings.temporalAlpha = 116 + scale8(context.controls.viscosity, 74);
  settings.blurX = 10 + scale8(context.controls.softness, 20);
  settings.blurY = 8 + scale8(context.controls.softness, 18);
  settings.floor = 6;
  settings.ceiling = 224;
  settings.contrast = 154;
  return settings;
}

static inline uint8_t anemoneSample(uint8_t h, uint8_t angle, uint8_t sample, const SceneContext& context) {
  const uint8_t depth = cylinderDepth8(sample);
  const uint8_t radius = cylinderRadius8(depth);
  const uint8_t pulsePhase = phase8(context.motion.risePhase);
  const uint8_t twistPhase = phase8(context.motion.rotationPhase);

  uint8_t thetaWarp = angle;
  thetaWarp += scale8(h, 55);
  thetaWarp += scale8(sin8_t(scale8(h, 81) + pulsePhase), 44) - 22;
  thetaWarp += scale8(depth, 14);

  const uint8_t petal = ease8InOutApprox(cos8_t(uint8_t(thetaWarp * 7U - twistPhase)));
  const uint8_t rootEnv = scale8(ease8InOutApprox(255 - h), qsub8(255, scale8(h, 112)));
  const uint8_t bodyEnv = bellCurve8(h, 56, 98);
  const uint8_t radialEnv = bellCurve8(radius, 184, 56);
  const uint8_t turb = wrappedCylinderNoiseFixed8(angle, h, 115 + scale8(depth, 45), 205, -(int32_t(context.motion.risePhase) >> 3));
  const uint8_t turbShape = qadd8(184, scale8(turb, 143));

  uint8_t field = scale8(petal, rootEnv);
  field = scale8(field, bodyEnv);
  field = scale8(field, radialEnv);
  field = scale8(field, turbShape);
  return field;
}

static void buildAnemoneField(SceneContext& context) {
  advanceMotion(context.motion, context.dt, anemoneMotionRates(context.controls));

  for (uint8_t y = 0; y < context.surface.height; y++) {
    const uint8_t h = cylinderHeight8(y, context.surface);
    const uint8_t breathe = 235 + scale8(sin8_t(scale8(h, 204) - phase8(context.motion.deformPhase)), 20);
    const uint8_t pulse = 209 + scale8(sin8_t(phase8(context.motion.anglePhase)), 46);

    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t angle = angle8(x, context.surface.width);
      uint32_t accumulated = 0;

      for (uint8_t sample = 0; sample < CYLINDER_DEPTH_SAMPLES; sample++) {
        addWeightedDepth(accumulated, anemoneSample(h, angle, sample, context), sample);
      }

      uint8_t energy = normalizeDepthEnergy(accumulated);
      energy = scale8(scale8(energy, pulse), breathe);
      context.field.raw[indexOf(x, y, context.surface)] = energy;
    }
  }
}

static inline CRGB anemonePalette(uint8_t value, const CRGB& low, const CRGB& mid, const CRGB& tip) {
  if (value < 82) return blendRgb(CRGB::Black, low, uint8_t((uint16_t(value) * 255U) / 82U));
  if (value < 170) return blendRgb(low, mid, uint8_t((uint16_t(value - 82) * 255U) / 88U));
  if (value < 230) return blendRgb(mid, tip, uint8_t((uint16_t(value - 170) * 255U) / 60U));
  return blendRgb(tip, CRGB(205, 255, 246), uint8_t((uint16_t(value - 230) * 255U) / 25U));
}

static void outputAnemone(SceneContext& context) {
  const CRGB low = colorOr(0, CRGB(4, 0, 30));
  const CRGB mid = colorOr(1, CRGB(142, 18, 172));
  const CRGB tip = colorOr(2, CRGB(76, 232, 218));

  for (uint8_t y = 0; y < context.surface.height; y++) {
    const uint8_t h = cylinderHeight8(y, context.surface);
    const uint8_t rootGlow = scale8(255 - h, 14);

    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t scalar = qadd8(context.field.blurred[indexOf(x, y, context.surface)], rootGlow);
      CRGB color = anemonePalette(ease8InOutApprox(scalar), low, mid, tip);
      color.nscale8_video(qadd8(8, scale8(scalar, 220)));
      limitColor(color, 210, 245, 245, 430);
      SEGMENT.setPixelColorXY(x, y, color);
    }
  }
}

static const SceneDefinition ANEMONE_SCENE = {
  buildAnemoneField,
  anemonePipelineSettings,
  outputAnemone
};

static void renderAnemone(RenderState& state, const Surface& surface, uint16_t dt) {
  renderScene(state, surface, dt, ANEMONE_SCENE);
}

} // namespace CylinderLamp
