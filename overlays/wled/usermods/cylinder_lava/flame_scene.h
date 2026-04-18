#pragma once

#include "cylinder_scene_contract.h"

namespace CylinderLamp {

static inline uint8_t circularDistance8(uint8_t a, uint8_t b) {
  int16_t distance = int16_t(a) - int16_t(b);
  if (distance < 0) distance = -distance;
  if (distance > 128) distance = 256 - distance;
  return uint8_t(distance);
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
  rates.angular = 2 + scale8(controls.flow, 4);
  rates.rise = 88 + scale8(controls.flow, 96);
  rates.rotation = 2 + scale8(controls.flow, 4);
  rates.deform = 2 + scale8(controls.softness, 4);
  return rates;
}

static PipelineSettings flamePipelineSettings(const SceneContext& context) {
  PipelineSettings settings;
  settings.temporalAlpha = 48 + scale8(context.controls.viscosity, 86);
  settings.blurX = 6 + scale8(context.controls.softness, 16);
  settings.blurY = 5 + scale8(context.controls.softness, 14);
  settings.floor = 10;
  settings.ceiling = 236;
  settings.contrast = 174;
  return settings;
}

static inline uint8_t flameRowCenter(const SceneContext& context, uint8_t h) {
  const uint8_t risePhase = phase8(context.motion.risePhase);
  const uint8_t spinPhase = phase8(context.motion.rotationPhase);
  const uint8_t deformPhase = phase8(context.motion.deformPhase);
  const int8_t rowLean = int8_t(scale8(sin8_t((h >> 1) + spinPhase), 14)) - 7;
  const int8_t risingLean = int8_t(scale8(sin8_t(h - risePhase + 63), 8)) - 4;
  const int8_t slowDrift = int8_t(scale8(cos8_t(deformPhase), 8)) - 4;
  return uint8_t(128 + rowLean + risingLean + slowDrift);
}

static inline uint8_t flameRowWidth(const SceneContext& context, uint8_t h) {
  const uint8_t risePhase = phase8(context.motion.risePhase);
  const uint8_t baseWidth = 72 + scale8(context.controls.size, 24);
  const uint8_t breathe = scale8(sin8_t(h - risePhase + 37), 10);
  uint8_t width = qsub8(qadd8(baseWidth, breathe), scale8(h, 54));
  if (width < 28) width = 28;
  return width;
}

static inline int8_t flameColumnDrift(const SceneContext& context, uint8_t h) {
  const uint8_t drift = sin8_t((h >> 1) + phase8(context.motion.rotationPhase));
  if (drift > 188) return 1;
  if (drift < 68) return -1;
  return 0;
}

static inline uint8_t flameIgnitionOpacity(uint8_t bottomDepth) {
  if (bottomDepth == 0) return 235;
  if (bottomDepth == 1) return 160;
  if (bottomDepth == 2) return 72;
  return 0;
}

static void buildFlameField(SceneContext& context) {
  advanceMotion(context.motion, context.dt, flameMotionRates(context.controls));

  const uint8_t transportAmount = 146 + scale8(context.controls.flow, 70);
  const uint8_t baseHeatAmount = 146 + scale8(context.controls.heat, 86);
  const uint8_t risePhase = phase8(context.motion.risePhase);
  const uint8_t ignitionPulse = qadd8(198, scale8(sin8_t(risePhase), 34));
  const uint8_t ignitionCenter = flameRowCenter(context, 0);
  const uint8_t ignitionWidth = 76 + scale8(context.controls.size, 22);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    const uint8_t h = heightFromBottom8(y, context.surface.height);
    const uint8_t center = flameRowCenter(context, h);
    const uint8_t rowWidth = flameRowWidth(context, h);
    const uint8_t bottomDepth = context.surface.height - 1 - y;
    const uint8_t ignitionOpacity = flameIgnitionOpacity(bottomDepth);
    const int8_t drift = flameColumnDrift(context, h);
    const uint8_t risingBand = scale8(sin8_t(h - risePhase), 34);

    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t angle = angle8(x, context.surface.width);
      const uint8_t bodyMask = flameTongue(angle, center, rowWidth);
      const uint8_t below = sampleWrapped(context.field.raw, x + drift, y + 1, context.surface);
      const uint8_t below2 = sampleWrapped(context.field.raw, x + drift, y + 2, context.surface);
      const uint8_t leftBelow = sampleWrapped(context.field.raw, x - 1, y + 1, context.surface);
      const uint8_t rightBelow = sampleWrapped(context.field.raw, x + 1, y + 1, context.surface);

      const uint16_t carriedSum = uint16_t(below) * 160U + uint16_t(below2) * 44U + uint16_t(leftBelow) * 20U + uint16_t(rightBelow) * 20U;
      uint8_t carried = uint8_t(carriedSum >> 8);
      carried = qsub8(carried, 5 + scale8(h, 42) + scale8(255 - bodyMask, 12));
      carried = scale8(carried, qadd8(34, scale8(bodyMask, 220)));
      addLayer(carried, scale8(risingBand, bodyMask), 34);

      const uint8_t current = context.field.raw[indexOf(x, y, context.surface)];
      uint8_t field = lerp8by8(current, carried, transportAmount);

      if (ignitionOpacity) {
        const uint8_t ignitionMask = flameTongue(angle, ignitionCenter, ignitionWidth);
        const uint8_t ignition = scale8(qadd8(118, scale8(ignitionMask, 118)), scale8(baseHeatAmount, ignitionPulse));
        addLayer(field, ignition, ignitionOpacity);
      }

      context.field.scratch[indexOf(x, y, context.surface)] = field;
    }
  }

  memcpy(context.field.raw, context.field.scratch, context.surface.count);
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
