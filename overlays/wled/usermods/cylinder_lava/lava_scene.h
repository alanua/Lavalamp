#pragma once

#include "cylinder_scene_contract.h"

namespace CylinderLamp {

struct Lobe {
  uint16_t x = 0;      // Q8 angular position around the cylinder
  uint16_t y = 0;      // Q8 logical row position
  uint16_t radius = 0; // Q8 pixel radius
  uint8_t weight = 0;
};

static inline CRGB lavaPalette(uint8_t value, const CRGB& deep, const CRGB& glow, const CRGB& core) {
  const CRGB shadow = blendRgb(deep, CRGB(34, 2, 0), 72);
  const CRGB ember = blendRgb(deep, glow, 54);
  const CRGB molten = blendRgb(glow, core, 76);

  if (value < 84) return blendRgb(deep, shadow, uint8_t((uint16_t(value) * 255U) / 84U));
  if (value < 154) return blendRgb(shadow, ember, uint8_t((uint16_t(value - 84) * 255U) / 70U));
  if (value < 214) return blendRgb(ember, glow, uint8_t((uint16_t(value - 154) * 255U) / 60U));
  if (value < 242) return blendRgb(glow, molten, uint8_t((uint16_t(value - 214) * 255U) / 28U));
  return blendRgb(molten, core, uint8_t((uint16_t(value - 242) * 255U) / 13U));
}

static inline uint8_t lavaLobeFalloff(uint8_t x, uint8_t y, const Surface& surface, const Lobe& lobe, uint8_t yStretch) {
  return radialFalloffQ8(x, y, surface, lobe.x, lobe.y, lobe.radius, yStretch);
}

static inline CRGB lavaColorOr(uint8_t slot, const CRGB& fallback, uint16_t maxTotal, uint8_t maxGreen, uint8_t maxBlue, bool rejectBright) {
  CRGB color = CRGB(SEGCOLOR(slot));
  uint16_t total = uint16_t(color.r) + color.g + color.b;
  if (total <= 4 || (rejectBright && total > maxTotal)) return fallback;

  if (color.g > maxGreen) color.g = maxGreen;
  if (color.b > maxBlue) color.b = maxBlue;
  if (color.r < color.g) color.r = qadd8(color.r, scale8(color.g - color.r, 96));

  total = uint16_t(color.r) + color.g + color.b;
  if (total > maxTotal) {
    color.nscale8_video(uint8_t((uint32_t(maxTotal) * 255U) / total));
  }
  return color;
}

static inline void limitLavaColor(CRGB& color) {
  limitColor(color, 238, 158, 18, 345);
}

static MotionRates lavaMotionRates(const SceneControls& controls) {
  MotionRates rates;
  rates.angular = 16 + scale8(controls.flow, 48);
  rates.rise = 12 + scale8(controls.flow, 36);
  rates.rotation = 10 + scale8(controls.heat, 18);
  rates.deform = 9 + scale8(controls.softness, 18);
  return rates;
}

static PipelineSettings lavaPipelineSettings(const SceneContext& context) {
  PipelineSettings settings;
  settings.temporalAlpha = 172 + scale8(context.controls.viscosity, 60);
  settings.blurX = 42 + scale8(context.controls.softness, 70);
  settings.blurY = 34 + scale8(context.controls.softness, 54);
  settings.floor = 0;
  settings.ceiling = 255;
  settings.contrast = 128;
  return settings;
}

static void buildLobes(const SceneContext& context, Lobe lobes[4]) {
  const uint16_t baseRadius = 820 + uint16_t(context.controls.size) * 3U;

  for (uint8_t i = 0; i < 4; i++) {
    const uint8_t phaseOffset = i * 59U;
    lobes[i].x = orbitXQ8(context.surface, context.motion, phaseOffset, 34);
    lobes[i].y = riseYQ8(context.surface, context.motion, i * 43U, 28);
    lobes[i].radius = baseRadius + uint16_t(sin8_t(phase8(context.motion.rotationPhase) + i * 71U + 37)) * 2U;
    lobes[i].weight = 92 + (i & 1 ? 18 : 0);
  }
}

static void buildLavaField(SceneContext& context) {
  advanceMotion(context.motion, context.dt, lavaMotionRates(context.controls));

  Lobe lobes[4];
  buildLobes(context, lobes);

  const uint8_t heatBias = 32 + scale8(context.controls.heat, 92);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    const uint8_t h = heightFromBottom8(y, context.surface.height);
    const uint8_t bottomHeat = scale8(255 - h, heatBias);
    const uint8_t verticalRoll = upwardDrift8(h, context.motion);

    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t a = angle8(x, context.surface.width);
      const uint8_t angularRoll = angularSwirl8(a, verticalRoll, context.motion, 28);
      const uint8_t localDeform = localDeform8(a, h, context.motion, 10);
      uint8_t field = qadd8(6, bottomHeat);
      addLayer(field, angularRoll, 28);
      addLayer(field, verticalRoll, 16);
      addLayer(field, localDeform, 18);

      for (uint8_t i = 0; i < 4; i++) {
        addLayer(field, lavaLobeFalloff(x, y, context.surface, lobes[i], 116), lobes[i].weight);
      }

      context.field.raw[indexOf(x, y, context.surface)] = field;
    }
  }
}

static void outputLava(SceneContext& context) {
  const CRGB deep = lavaColorOr(0, CRGB(3, 0, 1), 56, 10, 4, true);
  const CRGB glow = lavaColorOr(1, CRGB(224, 72, 0), 310, 104, 10, false);
  const CRGB core = lavaColorOr(2, CRGB(255, 138, 8), 390, 158, 18, false);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    const uint8_t h = heightFromBottom8(y, context.surface.height);
    const uint8_t bottomGlow = scale8(255 - h, 14);

    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t scalar = qadd8(context.field.blurred[indexOf(x, y, context.surface)], bottomGlow);
      const uint8_t safeScalar = scalar > 228 ? 228 : scalar;
      const uint8_t depthScalar = qsub8(safeScalar, 14);
      CRGB color = lavaPalette(ease8InOutApprox(depthScalar), deep, glow, core);
      color.nscale8_video(qadd8(12, scale8(depthScalar, 198)));
      limitLavaColor(color);
      SEGMENT.setPixelColorXY(x, y, color);
    }
  }
}

static const SceneDefinition LAVA_SCENE = {
  buildLavaField,
  lavaPipelineSettings,
  outputLava
};

static void renderLava(RenderState& state, const Surface& surface, uint16_t dt) {
  renderScene(state, surface, dt, LAVA_SCENE);
}

} // namespace CylinderLamp
