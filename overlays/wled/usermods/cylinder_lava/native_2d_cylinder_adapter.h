#pragma once

#include "wled.h"
#include "FX.h"

namespace CylinderLamp {

struct Native2DCylinderAdapter {
  static inline bool available() {
    return strip.isMatrix && SEGMENT.is2D();
  }

  static inline uint16_t width() {
    return SEGMENT.virtualWidth();
  }

  static inline uint16_t height() {
    return SEGMENT.virtualHeight();
  }

  static inline uint32_t now() {
    return strip.now;
  }

  static inline CRGB color(uint8_t slot) {
    return CRGB(SEGCOLOR(slot));
  }

  static inline void fill(CRGB color) {
    SEGMENT.fill(color);
  }

  static inline void setPixel(uint8_t x, uint8_t y, const CRGB& color) {
    SEGMENT.setPixelColorXY(x, y, color);
  }
};

} // namespace CylinderLamp
