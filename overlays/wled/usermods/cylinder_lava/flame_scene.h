#pragma once

#include "cylinder_scene_contract.h"

namespace CylinderLamp {

static inline uint8_t circularDistance8(uint8_t a, uint8_t b) {
  int16_t distance = int16_t(a) - int16_t(b);
  if (distance < 0) distance = -distance;
  if (distance > 128) distance = 256 - distance;
  return uint8_t(distance);
}

static inline int8_t circularOffset8(uint8_t angle, uint8_t center) {
  int16_t offset = int16_t(angle) - center;
  if (offset > 127) offset -= 256;
  if (offset < -128) offset += 256;
  return int8_t(offset);
}

static inline uint8_t localAngle8(uint8_t angle, uint8_t center) {
  return uint8_t(int16_t(circularOffset8(angle, center)) + 128);
}

static inline uint8_t flameTongue(uint8_t angle, uint8_t center, uint8_t width) {
  const uint8_t distance = circularDistance8(angle, center);
  if (distance >= width) return 0;
  return ease8InOutApprox(255 - uint8_t((uint16_t(distance) * 255U) / width));
}

static inline CRGB flameColorOr(uint8_t slot, const CRGB& fallback, uint16_t maxTotal, uint8_t maxGreen, uint8_t maxBlue) {
  CRGB color = CRGB(SEGCOLOR(slot));
  uint16_t total = uint16_t(color.r) + color.g + color.b;
  if (total <= 4 || total > maxTotal) return fallback;

  if (color.g > maxGreen) color.g = maxGreen;
  if (color.b > maxBlue) color.b = maxBlue;
  if (color.r < color.g) color.r = qadd8(color.r, scale8(color.g - color.r, 128));

  total = uint16_t(color.r) + color.g + color.b;
  if (total > maxTotal) {
    color.nscale8_video(uint8_t((uint32_t(maxTotal) * 255U) / total));
  }
  return color;
}

static inline CRGB flamePalette(uint8_t value, const CRGB& ember, const CRGB& body, const CRGB& core) {
  const CRGB deep = blendRgb(ember, CRGB(18, 0, 0), 96);
  const CRGB lower = blendRgb(ember, body, 80);
  const CRGB hot = blendRgb(body, core, 92);

  if (value < 70) return blendRgb(CRGB(0, 0, 0), deep, uint8_t((uint16_t(value) * 255U) / 70U));
  if (value < 148) return blendRgb(deep, lower, uint8_t((uint16_t(value - 70) * 255U) / 78U));
  if (value < 218) return blendRgb(lower, body, uint8_t((uint16_t(value - 148) * 255U) / 70U));
  return blendRgb(body, hot, uint8_t((uint16_t(value - 218) * 255U) / 37U));
}

static inline void limitFlameColor(CRGB& color) {
  limitColor(color, 244, 168, 24, 382);
}

static MotionRates flameMotionRates(const SceneControls& controls) {
  MotionRates rates;
  rates.angular = 2 + scale8(controls.flow, 5);
  rates.rise = 58 + scale8(controls.flow, 96);
  rates.rotation = 2 + scale8(controls.flow, 5);
  rates.deform = 2 + scale8(controls.softness, 5);
  return rates;
}

static PipelineSettings flamePipelineSettings(const SceneContext& context) {
  PipelineSettings settings;
  settings.temporalAlpha = 82 + scale8(context.controls.viscosity, 72);
  settings.blurX = 8 + scale8(context.controls.softness, 18);
  settings.blurY = 8 + scale8(context.controls.softness, 18);
  settings.floor = 8;
  settings.ceiling = 238;
  settings.contrast = 166;
  return settings;
}

static inline uint8_t flameRowCenter(const SceneContext& context, uint8_t h) {
  const uint8_t risePhase = phase8(context.motion.risePhase);
  const uint8_t spinPhase = phase8(context.motion.rotationPhase);
  const uint8_t deformPhase = phase8(context.motion.deformPhase);
  const int8_t rowLean = int8_t(scale8(sin8_t((h >> 1) + spinPhase), 12)) - 6;
  const int8_t flowLean = int8_t(scale8(sin8_t((h >> 2) - risePhase + deformPhase), 6)) - 3;
  const int8_t slowDrift = int8_t(scale8(cos8_t(deformPhase), 8)) - 4;
  return uint8_t(int16_t(rowLean) + flowLean + slowDrift);
}

static inline uint8_t flameRowWidth(const SceneContext& context, uint8_t h) {
  const uint8_t risePhase = phase8(context.motion.risePhase);
  const uint8_t deformPhase = phase8(context.motion.deformPhase);
  const uint8_t baseWidth = 54 + scale8(context.controls.size, 28);
  const uint8_t breathe = scale8(sin8_t((h >> 1) - risePhase + deformPhase), 10);
  uint8_t width = qsub8(qadd8(baseWidth, breathe), scale8(h, 34));
  if (width < 26) width = 26;
  return width;
}

static inline uint8_t flameVerticalBell(uint8_t value, uint8_t center, uint8_t radius) {
  int16_t distance = int16_t(value) - center;
  if (distance < 0) distance = -distance;
  if (distance >= radius) return 0;
  return ease8InOutApprox(255 - uint8_t((uint16_t(distance) * 255U) / radius));
}

static inline uint8_t flameFlowNoise(uint8_t h, uint8_t angle, uint8_t center, const MotionState& motion, const SceneControls& controls) {
  const uint8_t xScale = 18 + scale8(controls.size, 24);
  const uint8_t yScale = 38 + scale8(controls.flow, 34);
  const uint8_t localAngle = localAngle8(angle, center);
  const uint16_t xCoord = uint16_t(localAngle) * xScale + uint16_t(phase8(motion.rotationPhase)) * 7U;
  const uint16_t yCoord = uint16_t(h) * yScale - (motion.risePhase >> 1);
  const uint8_t noise = inoise8(xCoord, yCoord);
  return qadd8(58, scale8(ease8InOutApprox(noise), 122));
}

static inline uint8_t flameOrganicMotion(uint8_t h, uint8_t localAngle, const MotionState& motion) {
  const uint8_t softRadius = circularDistance8(localAngle, 128);
  const uint8_t nested = sin8_t((localAngle >> 1) + phase8(motion.deformPhase));
  const uint8_t wave = sin8_t(nested + (h >> 1) - phase8(motion.risePhase) + scale8(softRadius, 3));
  return scale8(ease8InOutApprox(wave), 24);
}

static inline uint8_t flameRadiantMotion(uint8_t h, uint8_t localAngle, const MotionState& motion) {
  const uint8_t lift = sin8_t(h + phase8(motion.risePhase));
  const uint8_t shimmer = sin8_t((h >> 1) + (localAngle >> 2) + phase8(motion.anglePhase));
  return scale8(avg8(lift, shimmer), 18);
}

static void buildFlameField(SceneContext& context) {
  advanceMotion(context.motion, context.dt, flameMotionRates(context.controls));

  const uint8_t bodyAmount = 132 + scale8(context.controls.heat, 62);
  const uint8_t coreAmount = 82 + scale8(context.controls.heat, 70);
  const uint8_t ignitionAmount = 82 + scale8(context.controls.heat, 84);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    const uint8_t h = heightFromBottom8(y, context.surface.height);
    const uint8_t center = flameRowCenter(context, h);
    const uint8_t rowWidth = flameRowWidth(context, h);
    const uint8_t verticalEnvelope = qsub8(255, scale8(h, 118));
    const uint8_t ignitionEnvelope = scale8(255 - h, 148);
    const uint8_t coreEnvelope = flameVerticalBell(h, 82, 96);

    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t angle = angle8(x, context.surface.width);
      const uint8_t bodyMask = flameTongue(angle, center, rowWidth);
      const uint8_t coreWidth = rowWidth > 20 ? rowWidth - 20 : rowWidth;
      const uint8_t coreMask = flameTongue(angle, center, coreWidth);
      const uint8_t flow = flameFlowNoise(h, angle, center, context.motion, context.controls);
      const uint8_t localAngle = localAngle8(angle, center);
      const uint8_t organicLife = flameOrganicMotion(h, localAngle, context.motion);
      const uint8_t radiantLife = flameRadiantMotion(h, localAngle, context.motion);

      uint8_t field = 0;
      addLayer(field, scale8(bodyMask, scale8(verticalEnvelope, qadd8(flow, organicLife))), bodyAmount);
      addLayer(field, scale8(coreMask, scale8(coreEnvelope, qadd8(138, qadd8(scale8(flow, 66), radiantLife)))), coreAmount);
      addLayer(field, scale8(bodyMask, ignitionEnvelope), ignitionAmount);

      context.field.raw[indexOf(x, y, context.surface)] = field;
    }
  }
}

static void outputFlame(SceneContext& context) {
  const CRGB ember = flameColorOr(0, CRGB(10, 0, 0), 72, 10, 4);
  const CRGB body = flameColorOr(1, CRGB(230, 62, 0), 318, 112, 10);
  const CRGB core = flameColorOr(2, CRGB(255, 148, 8), 410, 172, 20);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    const uint8_t h = heightFromBottom8(y, context.surface.height);
    const uint8_t baseGlow = scale8(255 - h, 18);

    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t scalar = qadd8(context.field.blurred[indexOf(x, y, context.surface)], baseGlow);
      const uint8_t safeScalar = scalar > 236 ? 236 : scalar;
      CRGB color = flamePalette(ease8InOutApprox(safeScalar), ember, body, core);
      color.nscale8_video(qadd8(10, scale8(safeScalar, 214)));
      limitFlameColor(color);
      SEGMENT.setPixelColorXY(x, y, color);
    }
  }
}

static const SceneDefinition FLAME_SCENE = {
  buildFlameField,
  flamePipelineSettings,
  outputFlame
};

static void renderFlame(RenderState& state, const Surface& surface, uint16_t dt) {
  renderScene(state, surface, dt, FLAME_SCENE);
}

} // namespace CylinderLamp
