#pragma once

#include "cylinder_geometry.h"

#ifndef CYLINDER_LAVA_MAX_PIXELS
#define CYLINDER_LAVA_MAX_PIXELS 256
#endif

namespace CylinderLamp {

struct FieldBuffers {
  uint8_t raw[CYLINDER_LAVA_MAX_PIXELS];
  uint8_t smooth[CYLINDER_LAVA_MAX_PIXELS];
  uint8_t scratch[CYLINDER_LAVA_MAX_PIXELS];
  uint8_t blurred[CYLINDER_LAVA_MAX_PIXELS];
};

struct PipelineSettings {
  uint8_t temporalAlpha = 200;
  uint8_t blurX = 64;
  uint8_t blurY = 48;
  uint8_t floor = 0;
  uint8_t ceiling = 255;
  uint8_t contrast = 128;
};

static inline void clearFields(FieldBuffers& fields) {
  memset(fields.raw, 0, sizeof(fields.raw));
  memset(fields.smooth, 0, sizeof(fields.smooth));
  memset(fields.scratch, 0, sizeof(fields.scratch));
  memset(fields.blurred, 0, sizeof(fields.blurred));
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

static inline void addLayer(uint8_t& target, uint8_t value, uint8_t opacity = 255) {
  target = qadd8(target, scale8(value, opacity));
}

static inline void limitColor(CRGB& color, uint8_t maxRed, uint8_t maxGreen, uint8_t maxBlue, uint16_t maxTotal) {
  if (color.r > maxRed) color.r = maxRed;
  if (color.g > maxGreen) color.g = maxGreen;
  if (color.b > maxBlue) color.b = maxBlue;

  const uint16_t total = uint16_t(color.r) + color.g + color.b;
  if (total > maxTotal) {
    color.nscale8_video(uint8_t((uint32_t(maxTotal) * 255U) / total));
  }
}

static void temporalSmooth(FieldBuffers& fields, const Surface& surface, uint8_t alpha) {
  const uint8_t rawAmount = 255 - alpha;

  for (uint16_t i = 0; i < surface.count; i++) {
    const uint16_t mixed = uint16_t(fields.smooth[i]) * alpha + uint16_t(fields.raw[i]) * rawAmount;
    fields.smooth[i] = uint8_t(mixed >> 8);
  }
}

static void spatialBlur(FieldBuffers& fields, const Surface& surface, uint8_t blurX, uint8_t blurY) {
  const uint8_t keepX = 255 - blurX;
  const uint8_t sideX = blurX >> 1;

  for (uint8_t y = 0; y < surface.height; y++) {
    for (uint8_t x = 0; x < surface.width; x++) {
      const uint8_t center = fields.smooth[indexOf(x, y, surface)];
      const uint8_t left = sampleWrapped(fields.smooth, x - 1, y, surface);
      const uint8_t right = sampleWrapped(fields.smooth, x + 1, y, surface);
      fields.scratch[indexOf(x, y, surface)] = qadd8(scale8(center, keepX), qadd8(scale8(left, sideX), scale8(right, sideX)));
    }
  }

  const uint8_t keepY = 255 - blurY;
  const uint8_t sideY = blurY >> 1;
  for (uint8_t y = 0; y < surface.height; y++) {
    const uint8_t up = y == 0 ? 0 : y - 1;
    const uint8_t down = y + 1 >= surface.height ? surface.height - 1 : y + 1;

    for (uint8_t x = 0; x < surface.width; x++) {
      const uint8_t center = fields.scratch[indexOf(x, y, surface)];
      const uint8_t above = fields.scratch[indexOf(x, up, surface)];
      const uint8_t below = fields.scratch[indexOf(x, down, surface)];
      fields.blurred[indexOf(x, y, surface)] = qadd8(scale8(center, keepY), qadd8(scale8(above, sideY), scale8(below, sideY)));
    }
  }
}

static inline uint8_t shapeScalar(uint8_t value, const PipelineSettings& settings) {
  if (value <= settings.floor) return 0;

  uint16_t shaped = value - settings.floor;
  const uint8_t range = settings.ceiling > settings.floor ? settings.ceiling - settings.floor : 1;
  shaped = (shaped * 255U) / range;
  if (shaped > 255U) shaped = 255U;

  if (settings.contrast != 128) {
    if (shaped >= 128U) {
      shaped = 128U + scale8(uint8_t(shaped - 128U), settings.contrast);
    } else {
      shaped = 128U - scale8(uint8_t(128U - shaped), settings.contrast);
    }
  }

  return uint8_t(shaped);
}

static void shapeField(FieldBuffers& fields, const Surface& surface, const PipelineSettings& settings) {
  if (settings.floor == 0 && settings.ceiling == 255 && settings.contrast == 128) return;

  for (uint16_t i = 0; i < surface.count; i++) {
    fields.blurred[i] = shapeScalar(fields.blurred[i], settings);
  }
}

static void runPipeline(FieldBuffers& fields, const Surface& surface, const PipelineSettings& settings) {
  temporalSmooth(fields, surface, settings.temporalAlpha);
  spatialBlur(fields, surface, settings.blurX, settings.blurY);
  shapeField(fields, surface, settings);
}

} // namespace CylinderLamp
