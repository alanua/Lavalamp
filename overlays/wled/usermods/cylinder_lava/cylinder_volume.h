#pragma once

#include <math.h>

#include "cylinder_scene_contract.h"

namespace CylinderLamp {

static constexpr uint8_t CY_DEPTH_SAMPLES = 4;
static constexpr float CY_PI = 3.14159265358979323846f;
static constexpr float CY_TWO_PI = 6.28318530717958647692f;
static constexpr float CY_DEPTH_K = 2.4f;
static constexpr float CY_NOISE_SCALE = 256.0f;

struct CyCoord {
  float theta;
  float h;
  float r;
};

static inline float cyClamp(float value, float low, float high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

static inline float cyFract(float value) {
  return value - floorf(value);
}

static inline float cyWrappedAngle(float angle) {
  while (angle > CY_PI) angle -= CY_TWO_PI;
  while (angle < -CY_PI) angle += CY_TWO_PI;
  return angle;
}

static inline float cySmoothstep(float edge0, float edge1, float value) {
  const float t = cyClamp((value - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}

static inline float cyGauss(float value, float center, float width) {
  const float x = (value - center) / width;
  return expf(-(x * x));
}

static inline float cyPow(float value, float exponent) {
  if (value <= 0.0f) return 0.0f;
  return powf(value, exponent);
}

static inline float cyDepth(uint8_t sample) {
  return float(sample) / float(CY_DEPTH_SAMPLES - 1);
}

static inline float cyDepthWeight(uint8_t sample) {
  return expf(-CY_DEPTH_K * cyDepth(sample));
}

static inline CyCoord cyCoord(uint8_t x, uint8_t y, uint8_t sample, const Surface& surface) {
  CyCoord coord;
  coord.theta = CY_TWO_PI * (float(x) / float(surface.width));
  coord.h = surface.height <= 1 ? 0.0f : float(y) / float(surface.height - 1);
  coord.r = 1.0f - cyDepth(sample);
  return coord;
}

static inline uint16_t cyNoiseCoord(float value) {
  return uint16_t(int32_t(value * CY_NOISE_SCALE));
}

static inline float cyNoise3(float x, float y, float z) {
  return float(inoise8(cyNoiseCoord(x), cyNoiseCoord(y), cyNoiseCoord(z))) / 255.0f;
}

static inline float cyCylinderNoise(
  float theta,
  float h,
  float r,
  float sx,
  float sy,
  float sz,
  float ox,
  float oy,
  float oz
) {
  return cyNoise3((cosf(theta) * r * sx) + ox, (h * sy) + oy, (sinf(theta) * r * sz) + oz);
}

static inline float cyHighlight(float energy) {
  return cyPow(cyClamp(energy, 0.0f, 1.0f), 1.6f);
}

static inline uint8_t cyEnergy8(float energy) {
  energy = cyClamp(energy, 0.0f, 1.0f);
  return uint8_t((energy * 255.0f) + 0.5f);
}

static inline CRGB cyColorOr(uint8_t slot, const CRGB& fallback) {
  return colorOr(slot, fallback);
}

static inline CRGB cyBlend3(float amount, const CRGB& low, const CRGB& mid, const CRGB& high) {
  amount = cyClamp(amount, 0.0f, 1.0f);
  if (amount < 0.5f) return blendRgb(low, mid, uint8_t(amount * 510.0f));
  return blendRgb(mid, high, uint8_t((amount - 0.5f) * 510.0f));
}

static inline void cyLimit(CRGB& color) {
  limitColor(color, 248, 248, 248, 520);
}

} // namespace CylinderLamp
