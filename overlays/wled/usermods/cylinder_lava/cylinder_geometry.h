#pragma once

#include "wled.h"
#include "FX.h"

namespace CylinderLamp {

struct Surface {
  uint8_t width = 0;
  uint8_t height = 0;
  uint16_t count = 0;
};

static inline uint16_t indexOf(uint8_t x, uint8_t y, const Surface& surface) {
  return uint16_t(y) * surface.width + x;
}

static inline uint8_t clampY(int16_t y, uint8_t height) {
  if (height == 0) return 0;
  if (y < 0) return 0;
  if (y >= height) return height - 1;
  return uint8_t(y);
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

static inline uint16_t xToQ8(uint8_t x) {
  return (uint16_t(x) << 8) + 128;
}

static inline uint16_t yToQ8(uint8_t y) {
  return (uint16_t(y) << 8) + 128;
}

static inline int32_t shortestXDeltaQ8(uint16_t a, uint16_t b, const Surface& surface) {
  const uint16_t circumference = uint16_t(surface.width) << 8;
  int32_t dx = int32_t(a) - int32_t(b);
  if (dx > int32_t(circumference / 2U)) dx -= circumference;
  if (dx < -int32_t(circumference / 2U)) dx += circumference;
  return dx;
}

static inline uint8_t heightFromBottom8(uint8_t y, uint8_t height) {
  if (height <= 1) return 0;
  return uint8_t(((uint16_t(height - 1 - y) * 255U) + ((height - 1) / 2U)) / uint16_t(height - 1));
}

static inline uint8_t sampleWrapped(const uint8_t* field, int16_t x, int16_t y, const Surface& surface) {
  return field[indexOf(wrapX(x, surface.width), clampY(y, surface.height), surface)];
}

static inline uint8_t radialFalloffQ8(
  uint8_t x,
  uint8_t y,
  const Surface& surface,
  uint16_t centerX,
  uint16_t centerY,
  uint16_t radius,
  uint8_t yStretch
) {
  const int32_t dx = abs(shortestXDeltaQ8(xToQ8(x), centerX, surface));
  int32_t dy = abs(int32_t(yToQ8(y)) - int32_t(centerY));
  dy = (dy * yStretch) >> 7;

  const uint32_t dist = uint32_t(dx * dx + dy * dy) >> 8;
  const uint32_t safeRadius = radius ? radius : 1;
  const uint32_t radiusSq = uint32_t(safeRadius * safeRadius) >> 8;
  if (radiusSq == 0 || dist >= radiusSq) return 0;

  uint32_t scaled = (dist * 255U) / radiusSq;
  if (scaled > 255U) scaled = 255U;
  return ease8InOutApprox(255U - uint8_t(scaled));
}

} // namespace CylinderLamp
