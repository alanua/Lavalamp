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
  const CRGB ember = blendRgb(deep, glow, 92);
  const CRGB molten = blendRgb(glow, core, 88);

  if (value < 72) return blendRgb(deep, ember, uint8_t((uint16_t(value) * 255U) / 72U));
  if (value < 166) return blendRgb(ember, glow, uint8_t((uint16_t(value - 72) * 255U) / 94U));
  if (value < 224) return blendRgb(glow, molten, uint8_t((uint16_t(value - 166) * 255U) / 58U));
  return blendRgb(molten, core, uint8_t((uint16_t(value - 224) * 255U) / 31U));
}

static inline uint8_t lobeFalloff(uint8_t x, uint8_t y, const Surface& surface, const Lobe& lobe, uint8_t yStretch) {
  return radialFalloffQ8(x, y, surface, lobe.x, lobe.y, lobe.radius, yStretch);
}

static void buildLobes(RenderState& state, const Surface& surface, Lobe lobes[4]) {
  const uint16_t baseRadius = 900 + uint16_t(SEGMENT.intensity) * 2U;

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

  const uint8_t heatBias = 44 + (SEGMENT.custom1 >> 2);
  const uint8_t rotation = uint8_t(state.rotationPhase >> 8);

  for (uint8_t y = 0; y < surface.height; y++) {
    const uint8_t h = heightFromBottom8(y, surface.height);
    const uint8_t bottomHeat = scale8(255 - h, heatBias);
    const uint8_t verticalRoll = sin8_t(h + uint8_t(state.liftPhase >> 8));

    for (uint8_t x = 0; x < surface.width; x++) {
      const uint8_t a = angle8(x, surface.width);
      const uint8_t angularRoll = sin8_t(a + rotation + scale8(verticalRoll, 28));
      uint8_t field = qadd8(18, bottomHeat);
      field = qadd8(field, scale8(angularRoll, 36));
      field = qadd8(field, scale8(verticalRoll, 22));

      for (uint8_t i = 0; i < 4; i++) {
        field = qadd8(field, scale8(lobeFalloff(x, y, surface, lobes[i], 116), lobes[i].weight));
      }

      state.raw[indexOf(x, y, surface)] = field;
    }
  }
}

static void outputLava(RenderState& state, const Surface& surface) {
  const CRGB deep = colorOr(0, CRGB(8, 0, 3));
  const CRGB glow = colorOr(1, CRGB(226, 63, 10));
  const CRGB core = colorOr(2, CRGB(255, 178, 68));

  for (uint8_t y = 0; y < surface.height; y++) {
    const uint8_t h = heightFromBottom8(y, surface.height);
    const uint8_t bottomGlow = scale8(255 - h, 26);

    for (uint8_t x = 0; x < surface.width; x++) {
      const uint8_t scalar = qadd8(state.blurred[indexOf(x, y, surface)], bottomGlow);
      CRGB color = lavaPalette(ease8InOutApprox(scalar), deep, glow, core);
      color.nscale8_video(qadd8(28, scale8(scalar, 205)));
      SEGMENT.setPixelColorXY(x, y, color);
    }
  }
}

static void render(RenderState& state, const Surface& surface, uint16_t dt) {
  const uint8_t speed = 1 + (SEGMENT.speed >> 6);
  state.anglePhase += dt * speed;
  state.liftPhase += dt * (1 + (SEGMENT.speed >> 7));
  state.rotationPhase += dt * (1 + (SEGMENT.custom1 >> 7));

  rawField(state, surface);
  temporalSmooth(state, surface);
  spatialBlur(state, surface);
  outputLava(state, surface);
}

} // namespace CylinderLava
