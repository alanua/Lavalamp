#pragma once

#include "wled.h"
#include "FX.h"

#ifndef CYLINDER_LAVA_MAX_PIXELS
#define CYLINDER_LAVA_MAX_PIXELS 256
#endif

namespace CylinderLava {

struct Surface {
  uint8_t width = 0;
  uint8_t height = 0;
  uint16_t count = 0;
};

struct Lobe {
  uint16_t x = 0;      // Q8 angular position around the cylinder
  uint16_t y = 0;      // Q8 logical row position
  uint16_t radius = 0; // Q8 pixel radius
  uint8_t weight = 0;
};

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

static inline uint16_t indexOf(uint8_t x, uint8_t y, const Surface& surface) {
  return uint16_t(y) * surface.width + x;
}

static inline uint8_t wrapX(int16_t x, uint8_t width) {
  if (width == 0) return 0;
  while (x < 0) x += width;
  while (x >= width) x -= width;
  return uint8_t(x);
}

static inline uint8_t angle8(uint8_t x, uint8_t width) {
  if (width == 0) return 0;
  return uint8_t((uint16_t(x) * 256U) / width);
}

static inline uint8_t heightFromBottom8(uint8_t y, uint8_t height) {
  if (height <= 1) return 0;
  return uint8_t(((uint16_t(height - 1 - y) * 255U) + ((height - 1) / 2U)) / uint16_t(height - 1));
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

static inline CRGB lavaPalette(uint8_t value, const CRGB& deep, const CRGB& glow, const CRGB& core) {
  const CRGB ember = blendRgb(deep, glow, 92);
  const CRGB molten = blendRgb(glow, core, 88);

  if (value < 72) return blendRgb(deep, ember, uint8_t((uint16_t(value) * 255U) / 72U));
  if (value < 166) return blendRgb(ember, glow, uint8_t((uint16_t(value - 72) * 255U) / 94U));
  if (value < 224) return blendRgb(glow, molten, uint8_t((uint16_t(value - 166) * 255U) / 58U));
  return blendRgb(molten, core, uint8_t((uint16_t(value - 224) * 255U) / 31U));
}

static inline uint8_t circularFalloff(uint8_t x, uint8_t y, const Surface& surface, const Lobe& lobe, uint8_t yStretch) {
  const uint16_t circumference = uint16_t(surface.width) << 8;
  const int32_t px = (uint16_t(x) << 8) + 128;
  const int32_t py = (uint16_t(y) << 8) + 128;

  int32_t dx = abs(px - int32_t(lobe.x));
  if (dx > int32_t(circumference / 2U)) dx = circumference - dx;

  int32_t dy = abs(py - int32_t(lobe.y));
  dy = (dy * yStretch) >> 7;

  const uint32_t dist = uint32_t(dx * dx + dy * dy) >> 8;
  const uint32_t radius = lobe.radius ? lobe.radius : 1;
  const uint32_t radiusSq = uint32_t(radius * radius) >> 8;
  if (radiusSq == 0 || dist >= radiusSq) return 0;

  uint32_t scaled = (dist * 255U) / radiusSq;
  if (scaled > 255U) scaled = 255U;
  return ease8InOutApprox(255U - uint8_t(scaled));
}

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
        field = qadd8(field, scale8(circularFalloff(x, y, surface, lobes[i], 116), lobes[i].weight));
      }

      state.raw[indexOf(x, y, surface)] = field;
    }
  }
}

static void temporalSmooth(RenderState& state, const Surface& surface) {
  const uint8_t alpha = 184 + scale8(SEGMENT.custom2, 48);
  const uint8_t rawAmount = 255 - alpha;

  for (uint16_t i = 0; i < surface.count; i++) {
    const uint16_t mixed = uint16_t(state.smooth[i]) * alpha + uint16_t(state.raw[i]) * rawAmount;
    state.smooth[i] = uint8_t(mixed >> 8);
  }
}

static void spatialBlur(RenderState& state, const Surface& surface) {
  const uint8_t blurX = 54 + (SEGMENT.custom3 >> 3);
  const uint8_t blurY = 42 + (SEGMENT.custom3 >> 4);
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
