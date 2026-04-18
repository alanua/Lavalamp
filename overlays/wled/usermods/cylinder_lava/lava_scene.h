#pragma once

#include "cylinder_pipeline.h"

namespace CylinderLava {

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

static inline uint8_t lobeFalloff(uint8_t x, uint8_t y, const Surface& surface, const Lobe& lobe, uint8_t yStretch) {
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
  if (color.r > 238) color.r = 238;
  if (color.g > 158) color.g = 158;
  if (color.b > 18) color.b = 18;

  const uint16_t maxTotal = 345;
  const uint16_t total = uint16_t(color.r) + color.g + color.b;
  if (total > maxTotal) {
    color.nscale8_video(uint8_t((uint32_t(maxTotal) * 255U) / total));
  }
}

static void buildLobes(RenderState& state, const Surface& surface, Lobe lobes[4]) {
  const uint16_t baseRadius = 820 + uint16_t(SEGMENT.intensity) * 3U;

  for (uint8_t i = 0; i < 4; i++) {
    const uint8_t phaseA = uint8_t(state.anglePhase >> 8) + i * 59U;
    const uint8_t phaseB = uint8_t(state.liftPhase >> 8) + i * 43U;
    const uint8_t phaseC = uint8_t(state.rotationPhase >> 8) + i * 71U;

    lobes[i].x = uint16_t(sin8_t(phaseA + scale8(sin8_t(phaseB), 34))) * surface.width;
    lobes[i].y = uint16_t(sin8_t(phaseB + scale8(cos8_t(phaseC), 28))) * (surface.height - 1);
    lobes[i].radius = baseRadius + uint16_t(sin8_t(phaseC + 37)) * 2U;
    lobes[i].weight = 92 + (i & 1 ? 18 : 0);
  }
}

static void rawField(RenderState& state, const Surface& surface) {
  Lobe lobes[4];
  buildLobes(state, surface, lobes);

  const uint8_t heatBias = 32 + scale8(SEGMENT.custom1, 92);
  const uint8_t rotation = uint8_t(state.rotationPhase >> 8);

  for (uint8_t y = 0; y < surface.height; y++) {
    const uint8_t h = heightFromBottom8(y, surface.height);
    const uint8_t bottomHeat = scale8(255 - h, heatBias);
    const uint8_t verticalRoll = sin8_t(h + uint8_t(state.liftPhase >> 8));

    for (uint8_t x = 0; x < surface.width; x++) {
      const uint8_t a = angle8(x, surface.width);
      const uint8_t angularRoll = sin8_t(a + rotation + scale8(verticalRoll, 28));
      uint8_t field = qadd8(6, bottomHeat);
      field = qadd8(field, scale8(angularRoll, 28));
      field = qadd8(field, scale8(verticalRoll, 16));

      for (uint8_t i = 0; i < 4; i++) {
        field = qadd8(field, scale8(lobeFalloff(x, y, surface, lobes[i], 116), lobes[i].weight));
      }

      state.raw[indexOf(x, y, surface)] = field;
    }
  }
}

static void outputLava(RenderState& state, const Surface& surface) {
  const CRGB deep = lavaColorOr(0, CRGB(3, 0, 1), 56, 10, 4, true);
  const CRGB glow = lavaColorOr(1, CRGB(224, 72, 0), 310, 104, 10, false);
  const CRGB core = lavaColorOr(2, CRGB(255, 138, 8), 390, 158, 18, false);

  for (uint8_t y = 0; y < surface.height; y++) {
    const uint8_t h = heightFromBottom8(y, surface.height);
    const uint8_t bottomGlow = scale8(255 - h, 14);

    for (uint8_t x = 0; x < surface.width; x++) {
      const uint8_t scalar = qadd8(state.blurred[indexOf(x, y, surface)], bottomGlow);
      const uint8_t safeScalar = scalar > 228 ? 228 : scalar;
      const uint8_t depthScalar = qsub8(safeScalar, 14);
      CRGB color = lavaPalette(ease8InOutApprox(depthScalar), deep, glow, core);
      color.nscale8_video(qadd8(12, scale8(depthScalar, 198)));
      limitLavaColor(color);
      SEGMENT.setPixelColorXY(x, y, color);
    }
  }
}

static void render(RenderState& state, const Surface& surface, uint16_t dt) {
  const uint8_t flow = 16 + scale8(SEGMENT.speed, 48);
  state.anglePhase += (uint32_t(dt) * flow) >> 4;
  state.liftPhase += (uint32_t(dt) * (12 + scale8(SEGMENT.speed, 36))) >> 4;
  state.rotationPhase += (uint32_t(dt) * (10 + scale8(SEGMENT.custom1, 18))) >> 4;

  rawField(state, surface);
  temporalSmooth(state, surface);
  spatialBlur(state, surface);
  outputLava(state, surface);
}

} // namespace CylinderLava
