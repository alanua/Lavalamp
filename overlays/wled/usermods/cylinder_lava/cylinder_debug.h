#pragma once

#include "cylinder_geometry.h"

namespace CylinderLava {

static void renderDebugPattern(const Surface& surface) {
  const uint8_t seamX = surface.width - 1;
  const uint8_t stripeX = surface.width / 2;
  const uint8_t stripeY = surface.height / 2;

  for (uint8_t y = 0; y < surface.height; y++) {
    for (uint8_t x = 0; x < surface.width; x++) {
      CRGB color = CRGB(3, 0, 0);
      if (x == 0) color = CRGB(180, 24, 0);
      if (x == seamX) color = CRGB(180, 112, 0);
      if (x == stripeX) color = CRGB(0, 130, 36);
      if (y == stripeY) color = CRGB(0, 48, 160);
      SEGMENT.setPixelColorXY(x, y, color);
    }
  }
}

} // namespace CylinderLava
