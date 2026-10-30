
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

struct CHSV { uint8_t h, s, v; CHSV(uint8_t a,uint8_t b,uint8_t c):h(a),s(b),v(c){} };

struct CRGB {
  static const CRGB Black;
  CRGB(const CHSV& hsv) {
    float h = hsv.h / 256.0f * 6; int sector = int(h); float f = h-sector;
    uint8_t p = hsv.v*(1-hsv.s/255.0f), q = hsv.v*(1-f*hsv.s/255.0f), t = hsv.v*(1-(1-f)*hsv.s/255.0f);
    switch(sector%6){case 0:r=hsv.v;g=t;b=p;break;case 1:r=q;g=hsv.v;b=p;break;case 2:r=p;g=hsv.v;b=t;break;case 3:r=p;g=q;b=hsv.v;break;case 4:r=t;g=p;b=hsv.v;break;default:r=hsv.v;g=p;b=q;}
  }
  uint8_t r;
  uint8_t g;
  uint8_t b;

  CRGB() : r(0), g(0), b(0) {}
  CRGB(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
  explicit CRGB(uint32_t packed)
      : r(uint8_t((packed >> 16) & 0xFF)),
        g(uint8_t((packed >> 8) & 0xFF)),
        b(uint8_t(packed & 0xFF)) {}

  void nscale8_video(uint8_t scale) {
    r = uint8_t((uint16_t(r) * scale + 255U) >> 8);
    g = uint8_t((uint16_t(g) * scale + 255U) >> 8);
    b = uint8_t((uint16_t(b) * scale + 255U) >> 8);
  }
};

static inline uint8_t qadd8(uint8_t a, uint8_t b) {
  const uint16_t sum = uint16_t(a) + b;
  return sum > 255U ? 255U : uint8_t(sum);
}

static inline uint8_t qsub8(uint8_t a, uint8_t b) {
  return a > b ? uint8_t(a - b) : uint8_t(0);
}

static inline uint8_t scale8(uint8_t value, uint8_t scale) {
  return uint8_t((uint16_t(value) * scale) >> 8);
}

static inline uint8_t lerp8by8(uint8_t a, uint8_t b, uint8_t amountOfB) {
  return uint8_t(a + ((int16_t(b) - a) * int16_t(amountOfB) >> 8));
}

static inline uint8_t avg8(uint8_t a, uint8_t b) {
  return uint8_t((uint16_t(a) + b + 1U) >> 1);
}

static inline uint8_t ease8InOutApprox(uint8_t value) {
  const float x = float(value) / 255.0f;
  const float eased = x * x * (3.0f - 2.0f * x);
  return uint8_t(std::clamp(eased * 255.0f, 0.0f, 255.0f));
}

static inline uint8_t sin8_t(uint8_t theta) {
  const float radians = (float(theta) / 256.0f) * 6.28318530717958647692f;
  return uint8_t(std::clamp((std::sin(radians) * 127.5f) + 128.0f, 0.0f, 255.0f));
}

static inline uint8_t cos8_t(uint8_t theta) {
  return sin8_t(uint8_t(theta + 64U));
}

struct SegmentStub {
  uint8_t speed = 96;
  uint8_t intensity = 128;
  uint8_t custom1 = 120;
  uint8_t custom2 = 150;
  uint8_t custom3 = 96;
  uint8_t width = 16;
  uint8_t height = 16;
  unsigned writes[256] = {};
  uint32_t colors[3] = {0x030001, 0xE04800, 0xFF8A08};
  CRGB pixels[256];

  bool is2D() const { return true; }
  uint16_t virtualWidth() const { return width; }
  uint16_t virtualHeight() const { return height; }

  void clearPixels() {
    for (CRGB& pixel : pixels) pixel = CRGB();
  }

  void setPixelColorXY(uint8_t x, uint8_t y, const CRGB& color) {
    const uint16_t index = uint16_t(y) * width + x;
    if (x >= width || y >= height) std::abort();
    if (index < 256U) { pixels[index] = color; ++writes[index]; }
  }
};

struct StripStub {
  bool isMatrix = true;
  uint32_t now = 0;
};

extern SegmentStub SEGMENT;
extern StripStub strip;

#define SEGCOLOR(slot) (SEGMENT.colors[(slot)])

inline const CRGB CRGB::Black = CRGB(0,0,0);
static inline float atan2_t(float y, float x) { return std::atan2(y,x); }
static inline long map(long x,long a,long b,long c,long d) { return (x-a)*(d-c)/(b-a)+c; }
#define SEGPALETTE 0
static inline CRGB ColorFromPalette(int, uint8_t index, uint8_t brightness) {
  return CRGB(CHSV(index,220,brightness));
}
// Smooth deterministic host noise substitute; production field code is unchanged.
// This harness checks renderer geometry/state, not FastLED bit-identical noise.
static inline uint8_t inoise8(uint16_t x,uint16_t y,uint16_t z=0) {
  const float v = 0.5f + 0.16f*std::sin(x*0.001f) + 0.17f*std::sin(y*0.0013f) + 0.16f*std::cos(z*0.0009f);
  return uint8_t(std::clamp(v*255,0.0f,255.0f));
}
