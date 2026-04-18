#pragma once

#include "cylinder_geometry.h"

#ifndef CYLINDER_LAVA_MAX_PIXELS
#define CYLINDER_LAVA_MAX_PIXELS 256
#endif

namespace CylinderLava {

struct RenderState {
  uint16_t width = 0;
  uint16_t height = 0;
  uint32_t lastMs = 0;
  uint16_t anglePhase = 0;
  uint16_t liftPhase = 0;
  uint16_t rotationPhase = 0;
  uint8_t initialized = 0;
  uint8_t raw[CYLINDER_LAVA_MAX_PIXELS];
  uint8_t smooth[CYLINDER_LAVA_MAX_PIXELS];
  uint8_t scratch[CYLINDER_LAVA_MAX_PIXELS];
  uint8_t blurred[CYLINDER_LAVA_MAX_PIXELS];
};

static bool prepare(RenderState& state, Surface& surface) {
  if (!strip.isMatrix || !SEGMENT.is2D()) return false;

  const uint16_t width = SEGMENT.virtualWidth();
  const uint16_t height = SEGMENT.virtualHeight();
  const uint32_t count = uint32_t(width) * height;
  if (width < 2 || height < 2 || count == 0 || count > CYLINDER_LAVA_MAX_PIXELS) return false;

  surface.width = uint8_t(width);
  surface.height = uint8_t(height);
  surface.count = uint16_t(count);

  if (!state.initialized || state.width != width || state.height != height) {
    memset(state.raw, 0, sizeof(state.raw));
    memset(state.smooth, 0, sizeof(state.smooth));
    memset(state.scratch, 0, sizeof(state.scratch));
    memset(state.blurred, 0, sizeof(state.blurred));
    state.width = width;
    state.height = height;
    state.lastMs = strip.now;
    state.anglePhase = 0x1200;
    state.liftPhase = 0x5800;
    state.rotationPhase = 0xA400;
    state.initialized = 1;
  }

  return true;
}

static uint16_t elapsedMs(RenderState& state) {
  const uint32_t now = strip.now;
  uint32_t dt = now - state.lastMs;
  state.lastMs = now;
  if (dt > 96) dt = 96;
  return uint16_t(dt);
}

static inline CRGB blendRgb(const CRGB& a, const CRGB& b, uint8_t amountOfB) {
  return CRGB(
    lerp8by8(a.r, b.r, amountOfB),
    lerp8by8(a.g, b.g, amountOfB),
    lerp8by8(a.b, b.b, amountOfB)
  );
}

static inline CRGB colorOr(uint8_t slot, const CRGB& fallback) {
  CRGB color = CRGB(SEGCOLOR(slot));
  if (color.r == 0 && color.g == 0 && color.b == 0) return fallback;
  return color;
}

static void temporalSmooth(RenderState& state, const Surface& surface) {
  const uint8_t alpha = 172 + scale8(SEGMENT.custom2, 60);
  const uint8_t rawAmount = 255 - alpha;

  for (uint16_t i = 0; i < surface.count; i++) {
    const uint16_t mixed = uint16_t(state.smooth[i]) * alpha + uint16_t(state.raw[i]) * rawAmount;
    state.smooth[i] = uint8_t(mixed >> 8);
  }
}

static void spatialBlur(RenderState& state, const Surface& surface) {
  const uint8_t blurX = 42 + scale8(SEGMENT.custom3, 70);
  const uint8_t blurY = 34 + scale8(SEGMENT.custom3, 54);
  const uint8_t keepX = 255 - blurX;
  const uint8_t sideX = blurX >> 1;

  for (uint8_t y = 0; y < surface.height; y++) {
    for (uint8_t x = 0; x < surface.width; x++) {
      const uint8_t center = state.smooth[indexOf(x, y, surface)];
      const uint8_t left = state.smooth[indexOf(wrapX(x - 1, surface.width), y, surface)];
      const uint8_t right = state.smooth[indexOf(wrapX(x + 1, surface.width), y, surface)];
      state.scratch[indexOf(x, y, surface)] = qadd8(scale8(center, keepX), qadd8(scale8(left, sideX), scale8(right, sideX)));
    }
  }

  const uint8_t keepY = 255 - blurY;
  const uint8_t sideY = blurY >> 1;
  for (uint8_t y = 0; y < surface.height; y++) {
    const uint8_t up = y == 0 ? 0 : y - 1;
    const uint8_t down = y + 1 >= surface.height ? surface.height - 1 : y + 1;

    for (uint8_t x = 0; x < surface.width; x++) {
      const uint8_t center = state.scratch[indexOf(x, y, surface)];
      const uint8_t above = state.scratch[indexOf(x, up, surface)];
      const uint8_t below = state.scratch[indexOf(x, down, surface)];
      state.blurred[indexOf(x, y, surface)] = qadd8(scale8(center, keepY), qadd8(scale8(above, sideY), scale8(below, sideY)));
    }
  }
}

} // namespace CylinderLava
