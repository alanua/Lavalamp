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
  rates.angular = 4 + scale8(controls.flow, 8);
  rates.rise = 44 + scale8(controls.flow, 76);
  rates.rotation = 3 + scale8(controls.flow, 8);
  rates.deform = 2 + scale8(controls.softness, 6);
  return rates;
}

static PipelineSettings flamePipelineSettings(const SceneContext& context) {
  PipelineSettings settings;
  settings.temporalAlpha = 118 + scale8(context.controls.viscosity, 76);
  settings.blurX = 8 + scale8(context.controls.softness, 20);
  settings.blurY = 8 + scale8(context.controls.softness, 18);
  settings.floor = 8;
  settings.ceiling = 230;
  settings.contrast = 168;
  return settings;
}

static void buildFlameField(SceneContext& context) {
  advanceMotion(context.motion, context.dt, flameMotionRates(context.controls));

  const uint8_t bodyWidth = 42 + scale8(context.controls.size, 18);
  const uint8_t baseHeatAmount = 66 + scale8(context.controls.heat, 88);
  const uint8_t bodyAmount = 150 + scale8(context.controls.heat, 62);
  const uint8_t coreAmount = 88 + scale8(context.controls.heat, 50);
  const uint8_t risePhase = phase8(context.motion.risePhase);
  const uint8_t spinPhase = phase8(context.motion.rotationPhase);
  const uint8_t deformPhase = phase8(context.motion.deformPhase);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    const uint8_t h = heightFromBottom8(y, context.surface.height);
    const uint8_t risingH = h - risePhase;
    const uint8_t verticalLife = qsub8(255, scale8(h, 174));
    const uint8_t baseHeat = scale8(255 - h, baseHeatAmount);
    const uint8_t lift = scale8(sin8_t(risingH), 36);
    const uint8_t taper = qadd8(54, scale8(verticalLife, 166));

    const int8_t bend = int8_t(scale8(sin8_t((h >> 1) + spinPhase), 12)) - 6;
    const uint8_t slowWaver = scale8(cos8_t((h >> 2) + deformPhase), 8);
    const uint8_t center = uint8_t(int16_t(spinPhase) + bend + slowWaver);
    const uint8_t coreWidth = bodyWidth > 22 ? bodyWidth - 22 : bodyWidth;

    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t angle = angle8(x, context.surface.width);
      const uint8_t body = flameTongue(angle, center, bodyWidth);
      const uint8_t core = flameTongue(angle, center, coreWidth);

      uint8_t field = qadd8(3, baseHeat);
      addLayer(field, scale8(body, qadd8(taper, lift)), bodyAmount);
      addLayer(field, scale8(core, qsub8(taper, 34)), coreAmount);

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
