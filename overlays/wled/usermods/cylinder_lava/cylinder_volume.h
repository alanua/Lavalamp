#pragma once

#include "cylinder_scene_contract.h"

namespace CylinderLamp {

static constexpr uint8_t CYLINDER_DEPTH_SAMPLES = 4;

enum CylinderDepthProfile : uint8_t {
  DEPTH_FADE_DEFAULT = 0,
  DEPTH_FADE_PLASMA = 1,
  DEPTH_FADE_DEEP = 2
};

static inline uint8_t cylinderHeight8(uint8_t y, const Surface& surface) {
  return heightFromBottom8(y, surface.height);
}

static inline uint8_t cylinderDepth8(uint8_t sample) {
  return sample >= CYLINDER_DEPTH_SAMPLES ? 255 : uint8_t(sample * 85U);
}

static inline uint8_t cylinderRadius8(uint8_t depth) {
  return 255 - depth;
}

static inline uint8_t bellCurve8(uint8_t value, uint8_t center, uint8_t radius) {
  int16_t distance = int16_t(value) - center;
  if (distance < 0) distance = -distance;
  if (distance >= radius) return 0;
  return ease8InOutApprox(255 - uint8_t((uint16_t(distance) * 255U) / radius));
}

static inline uint8_t depthFade8(uint8_t sample, CylinderDepthProfile profile = DEPTH_FADE_DEFAULT) {
  static const uint8_t defaultFade[CYLINDER_DEPTH_SAMPLES] = {255, 114, 52, 23};
  static const uint8_t plasmaFade[CYLINDER_DEPTH_SAMPLES] = {255, 127, 63, 31};
  static const uint8_t deepFade[CYLINDER_DEPTH_SAMPLES] = {255, 104, 42, 17};

  if (sample >= CYLINDER_DEPTH_SAMPLES) sample = CYLINDER_DEPTH_SAMPLES - 1;
  if (profile == DEPTH_FADE_PLASMA) return plasmaFade[sample];
  if (profile == DEPTH_FADE_DEEP) return deepFade[sample];
  return defaultFade[sample];
}

static inline uint16_t depthFadeTotal(CylinderDepthProfile profile = DEPTH_FADE_DEFAULT) {
  uint16_t total = 0;
  for (uint8_t i = 0; i < CYLINDER_DEPTH_SAMPLES; i++) total += depthFade8(i, profile);
  return total ? total : 1;
}

static inline uint8_t normalizeDepthEnergy(uint32_t accumulated, CylinderDepthProfile profile = DEPTH_FADE_DEFAULT) {
  const uint32_t total = depthFadeTotal(profile);
  accumulated = (accumulated + (total >> 1)) / total;
  return accumulated > 255U ? 255 : uint8_t(accumulated);
}

static inline uint16_t centeredNoiseCoord8(uint8_t wave, uint16_t scale, int32_t offset = 32768L) {
  const int16_t centered = int16_t(wave) - 128;
  return uint16_t(offset + int32_t(centered) * int32_t(scale));
}

static inline uint8_t wrappedCylinderNoise8(
  uint8_t angle,
  uint8_t height,
  uint8_t radius,
  uint16_t angularScale,
  uint16_t verticalScale,
  int32_t yPhase,
  int32_t zPhase = 0
) {
  const uint16_t radialScale = scale8(radius, angularScale);
  const uint16_t nx = centeredNoiseCoord8(cos8_t(angle), radialScale);
  const uint16_t nz = centeredNoiseCoord8(sin8_t(angle), radialScale, 32768L + zPhase);
  const uint16_t ny = uint16_t(uint32_t(height) * verticalScale + yPhase);
  return inoise8(nx, ny, nz);
}

static inline uint8_t wrappedCylinderNoiseFixed8(
  uint8_t angle,
  uint8_t height,
  uint16_t angularScale,
  uint16_t verticalScale,
  int32_t yPhase,
  int32_t zPhase = 0
) {
  const uint16_t nx = centeredNoiseCoord8(cos8_t(angle), angularScale);
  const uint16_t nz = centeredNoiseCoord8(sin8_t(angle), angularScale, 32768L + zPhase);
  const uint16_t ny = uint16_t(uint32_t(height) * verticalScale + yPhase);
  return inoise8(nx, ny, nz);
}

static inline void addWeightedDepth(uint32_t& target, uint8_t value, uint8_t sample, CylinderDepthProfile profile = DEPTH_FADE_DEFAULT) {
  target += uint32_t(value) * depthFade8(sample, profile);
}

static inline CRGB palette3(uint8_t value, const CRGB& low, const CRGB& mid, const CRGB& high) {
  if (value < 96) return blendRgb(CRGB::Black, low, uint8_t((uint16_t(value) * 255U) / 96U));
  if (value < 190) return blendRgb(low, mid, uint8_t((uint16_t(value - 96) * 255U) / 94U));
  return blendRgb(mid, high, uint8_t((uint16_t(value - 190) * 255U) / 65U));
}

} // namespace CylinderLamp
