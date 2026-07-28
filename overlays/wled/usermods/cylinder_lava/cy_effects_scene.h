#pragma once

#include "cylinder_volume.h"

namespace CylinderLamp {

enum CyEffectKind : uint8_t {
  CY_EFFECT_ANEMONE = 0,
  CY_EFFECT_LAVA_LAMP,
  CY_EFFECT_FLAME,
  CY_EFFECT_PLASMA_CORE,
  CY_EFFECT_DEEP_NOISE,
  CY_EFFECT_AURORA_TUBE,
  CY_EFFECT_INNER_SWIRL,
  CY_EFFECT_BUBBLES_VOLUME,
  CY_EFFECT_RING_RIPPLES,
  CY_EFFECT_RING_RIPPLES_RAINBOW,
  CY_EFFECT_BOTTOM_RAYS,
  CY_EFFECT_RISING_BANDS,
  CY_EFFECT_HELICAL_PLASMA,
  CY_EFFECT_NOISE_WAVES_TUBE,
  CY_EFFECT_CELL_MEMBRANE_FLOW,
  CY_EFFECT_CROSS_BANDS_TUBE
};

static PipelineSettings cyExactPipelineSettings(const SceneContext&) {
  PipelineSettings settings;
  settings.temporalAlpha = 0;
  settings.blurX = 0;
  settings.blurY = 0;
  settings.floor = 0;
  settings.ceiling = 255;
  settings.contrast = 128;
  return settings;
}

static inline float cySoftBand(float value, float center, float halfWidth) {
  const float x = cyClamp(1.0f - (fabsf(value - center) / halfWidth), 0.0f, 1.0f);
  return x * x * (3.0f - (2.0f * x));
}

static inline uint8_t cyRuntimeDepthSamples(CyEffectKind) {
  return 1;
}

static inline float cyOpticalTransfer(float e) {
  if (e < 0.0f) e = 0.0f;

  // kill low-level wash that the diffuser turns into pale fog
  e = (e - 0.12f) / 0.88f;
  if (e < 0.0f) e = 0.0f;
  if (e > 1.0f) e = 1.0f;

  // lift readable shape energy
  e = powf(e, 0.72f);

  // stronger separation for active shapes
  if (e > 0.28f) {
    e = 0.28f + (e - 0.28f) * 1.65f;
  }

  if (e > 1.0f) e = 1.0f;
  return e;
}

struct FxCoord {
  float x;
  float y;
};

struct OctopusSample {
  uint8_t angle;
  uint8_t radius;
};

static inline float cyWrapPi(float a) {
  while (a > CY_PI) a -= CY_TWO_PI;
  while (a < -CY_PI) a += CY_TWO_PI;
  return a;
}

static inline FxCoord fxCoordFlatCentered(int x, int y, int W, int H) {
  FxCoord c;
  c.x = float(x) - (0.5f * float(W));
  c.y = float(y) - (0.5f * float(H));
  return c;
}

static inline float octopusScaleFromUi(uint8_t value) {
  return 0.5f + ((float(value) / 255.0f) * 2.5f);
}

static inline FxCoord fxCoordCylinderShell(int x, int y, int W, int H, float theta0, float h0, float scaleX, float scaleY) {
  const float u = (float(x) + 0.5f) / float(W);
  const float h = H <= 1 ? 0.0f : float(y) / float(H - 1);
  const float theta = CY_TWO_PI * u;
  const float a = cyWrapPi(theta - theta0);

  FxCoord c;
  c.x = a * (float(W) / CY_TWO_PI) * scaleX;
  c.y = (h - h0) * float(H) * scaleY;
  return c;
}

static inline OctopusSample octopusSampleFromCoord(const FxCoord& c, int W, int H) {
  const uint8_t mapp = 180U / uint8_t(W > H ? W : H);

  OctopusSample sample;
  sample.angle = uint8_t(int(40.7436f * atan2_t(c.y, c.x)));
  sample.radius = uint8_t(cyClamp(sqrtf((c.x * c.x) + (c.y * c.y)) * float(mapp), 0.0f, 255.0f));
  return sample;
}

static inline uint16_t octopusStep(float t) {
  const uint16_t speed = uint16_t(SEGMENT.speed / 32U) + 1U;
  return uint16_t(t * 0.04f * float(speed));
}

static inline CRGB octopusKernel(const OctopusSample& sample, uint16_t step, uint8_t legsControl) {
  const uint8_t halfStep = uint8_t(step >> 1);
  const uint8_t step8 = uint8_t(step);
  const uint8_t legs = uint8_t(legsControl / 4U) + 1U;
  const uint8_t inner = sin8_t(uint8_t(((int(sample.angle) * 4 - int(sample.radius)) / 4) + int(halfStep)));
  uint16_t intensity = sin8_t(uint8_t(int(inner) + int(sample.radius) - int(step8) + (int(sample.angle) * int(legs))));
  intensity = uint16_t(map((intensity * intensity) & 0xFFFF, 0, 65535, 0, 255));
  return ColorFromPalette(SEGPALETTE, uint8_t(halfStep - sample.radius), uint8_t(intensity));
}

static float cyFieldLavaLamp(const CyCoord& c, float t) {
  const float liquid =
      0.008f +
      (0.010f * c.h) +
      (0.006f * expf(-((c.r - 0.82f) * (c.r - 0.82f)) / 0.030f));

  const float resY = expf(-((c.h - 0.080f) * (c.h - 0.080f)) / 0.0060f);
  const float resOuter = expf(-((c.r - 0.82f) * (c.r - 0.82f)) / 0.012f);
  const float resInner = expf(-((c.r - 0.58f) * (c.r - 0.58f)) / 0.050f);
  const float reservoirPulse = 0.94f + 0.06f * sinf(0.00075f * t);
  const float reservoir = 1.55f * resY * ((0.74f * resOuter) + (0.26f * resInner)) * reservoirPulse;

  float blobs = 0.0f;
  for (uint8_t k = 0; k < 3; k++) {
    const float kf = float(k);

    const float thetaBase =
        (k == 0) ? 0.20f :
        (k == 1) ? 2.55f :
                   4.55f;

    const float thetaCenter =
        thetaBase +
        (0.18f * sinf((0.00013f * t) + (1.1f * kf))) +
        (0.06f * sinf((0.00031f * t) + (0.7f * kf)));

    float dtheta = c.theta - thetaCenter;
    while (dtheta > 3.14159265f) dtheta -= 6.28318531f;
    while (dtheta < -3.14159265f) dtheta += 6.28318531f;

    const float hCenter =
        (k == 0) ? (0.24f + 0.06f * sinf(0.00020f * t + 0.0f)) :
        (k == 1) ? (0.47f + 0.07f * sinf(0.00017f * t + 1.7f)) :
                   (0.69f + 0.06f * sinf(0.00016f * t + 3.1f));

    const float rCenter =
        (k == 0) ? 0.67f :
        (k == 1) ? 0.61f :
                   0.65f;

    const float angW =
        (k == 0) ? 0.48f :
        (k == 1) ? 0.40f :
                   0.36f;

    const float hW =
        (k == 0) ? 0.15f :
        (k == 1) ? 0.16f :
                   0.14f;

    const float rW =
        (k == 0) ? 0.22f :
        (k == 1) ? 0.23f :
                   0.20f;

    const float blob =
        expf(-(dtheta * dtheta) / (angW * angW)) *
        expf(-((c.h - hCenter) * (c.h - hCenter)) / (hW * hW)) *
        expf(-((c.r - rCenter) * (c.r - rCenter)) / (rW * rW));

    blobs += 1.08f * blob;
  }

  const float heat = 0.22f * expf(-c.h / 0.18f) * expf(-((c.r - 0.70f) * (c.r - 0.70f)) / 0.040f);

  const float waxRaw = reservoir + blobs + heat;

  float wax = (waxRaw - 0.40f) / 0.22f;
  if (wax < 0.0f) wax = 0.0f;
  if (wax > 1.0f) wax = 1.0f;
  wax = wax * wax * (3.0f - 2.0f * wax);

  const float bottomGlow =
      0.12f *
      expf(-c.h / 0.14f) *
      ((0.68f * resOuter) + (0.32f * resInner));

  float f = liquid + (1.55f * wax) + bottomGlow;

  if (f < 0.0f) f = 0.0f;
  f *= 1.30f;
  if (f > 1.0f) f = 1.0f;

  return f;
}

static float cyFieldFlame(const CyCoord& c, float t) {
  const float liquid = 0.006f;
  const float bodyCenter = 0.35f * sinf(0.00055f * t);
  float dtheta = cyWrappedAngle(c.theta - bodyCenter);

  const float source =
      1.65f *
      expf(-((c.h - 0.055f) * (c.h - 0.055f)) / 0.0025f) *
      expf(-(dtheta * dtheta) / 0.200f) *
      expf(-((c.r - 0.78f) * (c.r - 0.78f)) / 0.032f);

  const float coreCenter =
      bodyCenter +
      (0.12f * sinf((2.8f * c.h) - (0.00135f * t))) +
      (0.04f * sinf((7.0f * c.h) - (0.0020f * t)));
  const float coreTheta = cyWrappedAngle(c.theta - coreCenter);
  const float taper = expf(-c.h / 0.42f);
  const float core =
      1.28f *
      expf(-(coreTheta * coreTheta) / 0.105f) *
      expf(-((c.r - 0.74f) * (c.r - 0.74f)) / 0.030f) *
      taper;

  float tongues = 0.0f;
  for (uint8_t i = 0; i < 3; i++) {
    const float fi = float(i);
    const float phase = 2.0943951f * fi;
    const float tongueCenter =
        bodyCenter +
        (0.22f * sinf((3.6f * c.h) - (0.00155f * t) + phase));
    const float td = cyWrappedAngle(c.theta - tongueCenter);
    const float vertical =
        expf(-((c.h - (0.22f + (0.15f * fi))) * (c.h - (0.22f + (0.15f * fi)))) / 0.040f);
    const float radial = expf(-((c.r - 0.82f) * (c.r - 0.82f)) / 0.026f);
    tongues += 0.52f * expf(-(td * td) / 0.070f) * vertical * radial;
  }

  const float upperFade = expf(-c.h / 0.55f);
  float f = liquid + source + (core * upperFade) + tongues;
  if (f < 0.0f) f = 0.0f;
  f *= 1.30f;
  if (f > 1.0f) f = 1.0f;
  return f;
}

static float cyFieldPlasmaCore(const CyCoord& c, float t) {
  const float core = 1.15f * cyGauss(c.r, 0.30f, 0.16f) * cyGauss(c.h, 0.52f, 0.30f);
  const float a1 = 0.5f + 0.5f * sinf((3.0f * c.theta) + (4.2f * c.h) - (0.0018f * t));
  const float a2 = 0.5f + 0.5f * sinf((5.0f * c.theta) - (3.4f * c.h) + (0.0013f * t));
  const float arc = 0.58f * a1 + 0.42f * a2;
  const float nP = cyCylinderNoise(c.theta, c.h, c.r, 2.4f, 4.5f, 2.4f, 0.0f, 0.0007f * t, 0.0f);
  const float shell = cyGauss(c.r, 0.78f, 0.14f);
  return 0.82f * core + 1.05f * cyPow(arc, 1.8f) * shell * (0.7f + 0.6f * nP);
}

static float cyFieldDeepNoise(const CyCoord& c, float t) {
  const float n1 = cyCylinderNoise(c.theta, c.h, c.r, 1.8f, 3.5f, 1.8f, 0.0f, -0.0006f * t, 0.0f);
  const float n2 = cyCylinderNoise(c.theta, c.h, c.r, 3.6f, 7.0f, 3.6f, 103.0f, -0.0009f * t, 57.0f);
  const float n = 0.70f * n1 + 0.30f * n2;
  const float dens = 0.80f + 0.20f * cyGauss(c.h, 0.35f, 0.42f);
  const float body = cyGauss(c.r, 0.64f, 0.26f);
  return n * dens * body;
}

static float cyFieldAuroraTube(const CyCoord& c, float t) {
  const float curtain = 0.5f + 0.5f * sinf((2.2f * c.theta) + (5.4f * c.h) - (0.0008f * t));
  const float shimmer = 0.5f + 0.5f * sinf((1.3f * c.theta) - (3.0f * c.h) + (0.00035f * t));
  const float upper = 0.75f + 0.25f * c.h;
  const float rad = cyGauss(c.r, 0.68f, 0.24f);
  return (curtain * curtain) * (0.70f + 0.35f * shimmer) * upper * rad;
}

static float cyFieldInnerSwirl(const CyCoord& c, float t) {
  const float swirl = 0.5f + 0.5f * sinf((4.2f * c.theta) + (7.0f * c.h) - (0.0010f * t));
  const float body = cyGauss(c.r, 0.58f, 0.24f);
  return swirl * body;
}

static float cyFieldBubblesVolume(const CyCoord& c, float t) {
  const float nB1 = 0.5f + 0.5f * sinf((2.2f * c.theta) + (3.8f * c.h) - (0.0003f * t));
  const float nB2 = 0.5f + 0.5f * cosf((4.4f * c.theta) - (2.9f * c.h) + (0.0004f * t));
  const float cell = fabsf(nB1 - nB2);
  const float body = cyGauss(c.r, 0.66f, 0.24f);
  return (cell * (0.65f + 0.35f * cell)) * body;
}

static float cyFieldRingRipple(const CyCoord& c, float t) {
  const float p = cyFract(0.00022f * t);
  const float da = cyWrappedAngle(c.theta - 1.8f) / 1.2f;
  const float dh = (c.h - p) / 0.18f;
  const float dr = sqrtf((da * da) + (dh * dh));
  const float wave = cyGauss(dr, 1.0f, 0.22f);
  const float rad = cyGauss(c.r, 0.72f, 0.18f);
  return wave * rad;
}

static float cyFieldBottomRays(const CyCoord& c, float t) {
  const float rays = 0.5f + 0.5f * cosf((8.0f * c.theta) - (0.0009f * t));
  const float root = expf(-c.h / 0.24f);
  const float rad = cyGauss(c.r, 0.70f, 0.22f);
  return cyPow(rays, 1.7f) * root * rad;
}

static float cyFieldRisingBands(const CyCoord& c, float t) {
  const float bands = 0.5f + 0.5f * sinf((6.0f * c.theta) + (10.0f * c.h) - (0.0015f * t));
  const float drift = 0.5f + 0.5f * cosf((1.8f * c.theta) - (4.0f * c.h) + (0.0011f * t));
  const float rad = cyGauss(c.r, 0.74f, 0.18f);
  return (bands * (0.75f + 0.25f * bands)) * (0.75f + 0.35f * drift) * rad;
}

static float cyFieldHelicalPlasma(const CyCoord& c, float t) {
  const float h1 = 0.5f + 0.5f * sinf((5.0f * c.theta) + (6.5f * c.h) - (0.0012f * t));
  const float h2 = 0.5f + 0.5f * sinf((3.0f * c.theta) - (4.5f * c.h) + (0.0009f * t));
  const float body = cyGauss(c.r, 0.64f, 0.22f);
  return (0.62f * h1 + 0.38f * h2) * body;
}

static float cyFieldNoiseWavesTube(const CyCoord& c, float t) {
  const float wave = 0.5f + 0.5f * sinf((2.6f * c.theta) + (8.0f * c.h) - (0.0008f * t));
  const float nNW = cyCylinderNoise(c.theta, c.h, c.r, 2.0f, 4.0f, 2.0f, 0.0f, -0.0005f * t, 0.0f);
  const float body = cyGauss(c.r, 0.68f, 0.24f);
  return wave * (0.70f + 0.40f * nNW) * body;
}

static float cyFieldCellMembraneFlow(const CyCoord& c, float t) {
  const float nC1 = 0.5f + 0.5f * sinf((2.4f * c.theta) + (4.4f * c.h) - (0.0004f * t));
  const float nC2 = 0.5f + 0.5f * cosf((4.8f * c.theta) - (3.8f * c.h) + (0.0006f * t));
  const float mem = 1.0f - fabsf(nC1 - nC2);
  const float body = cyGauss(c.r, 0.66f, 0.24f);
  return (mem * mem) * body;
}

static float cyFieldCrossBandsTube(const CyCoord& c, float t) {
  const float c1 = 0.5f + 0.5f * sinf((6.0f * c.theta) + (8.0f * c.h) - (0.0011f * t));
  const float c2 = 0.5f + 0.5f * sinf((-4.0f * c.theta) + (7.0f * c.h) + (0.0009f * t));
  const float body = cyGauss(c.r, 0.70f, 0.22f);
  return (c1 * c2) * body;
}

static float cyField(CyEffectKind kind, const CyCoord& c, float t) {
  switch (kind) {
    case CY_EFFECT_ANEMONE: return 0.0f;
    case CY_EFFECT_LAVA_LAMP: return cyFieldLavaLamp(c, t);
    case CY_EFFECT_FLAME: return cyFieldFlame(c, t);
    case CY_EFFECT_PLASMA_CORE: return cyFieldPlasmaCore(c, t);
    case CY_EFFECT_DEEP_NOISE: return cyFieldDeepNoise(c, t);
    case CY_EFFECT_AURORA_TUBE: return cyFieldAuroraTube(c, t);
    case CY_EFFECT_INNER_SWIRL: return cyFieldInnerSwirl(c, t);
    case CY_EFFECT_BUBBLES_VOLUME: return cyFieldBubblesVolume(c, t);
    case CY_EFFECT_RING_RIPPLES:
    case CY_EFFECT_RING_RIPPLES_RAINBOW: return cyFieldRingRipple(c, t);
    case CY_EFFECT_BOTTOM_RAYS: return cyFieldBottomRays(c, t);
    case CY_EFFECT_RISING_BANDS: return cyFieldRisingBands(c, t);
    case CY_EFFECT_HELICAL_PLASMA: return cyFieldHelicalPlasma(c, t);
    case CY_EFFECT_NOISE_WAVES_TUBE: return cyFieldNoiseWavesTube(c, t);
    case CY_EFFECT_CELL_MEMBRANE_FLOW: return cyFieldCellMembraneFlow(c, t);
    case CY_EFFECT_CROSS_BANDS_TUBE: return cyFieldCrossBandsTube(c, t);
  }
  return 0.0f;
}

static void buildCyField(SceneContext& context, CyEffectKind kind) {
  advanceMotion(context.motion, context.dt, MotionRates());
  const float t = float(strip.now);
  const uint8_t depthSamples = cyRuntimeDepthSamples(kind);
  float totalWeight = 0.0f;
  for (uint8_t sample = 0; sample < depthSamples; sample++) totalWeight += cyDepthWeight(sample);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    for (uint8_t x = 0; x < context.surface.width; x++) {
      float accumulated = 0.0f;

      for (uint8_t sample = 0; sample < depthSamples; sample++) {
        const CyCoord c = cyCoord(x, y, sample, context.surface);
        accumulated += cyDepthWeight(sample) * cyField(kind, c, t);
      }

      context.field.raw[indexOf(x, y, context.surface)] = cyEnergy8(accumulated / totalWeight);
    }
  }
}

static CRGB cyColor(CyEffectKind kind, float energy, float theta, float t) {
  const float h = cyHighlight(energy);
  const uint8_t e8 = cyEnergy8(energy);
  const uint8_t h8 = cyEnergy8(h);

  switch (kind) {
    case CY_EFFECT_ANEMONE:
      return cyBlend3(
        h,
        CRGB(0, 0, 0),
        CRGB(95, 16, 180),
        CRGB(100, 245, 255)
      );
    case CY_EFFECT_LAVA_LAMP:
      return cyBlend3(
        h,
        CRGB(0, 0, 0),
        CRGB(210, 46, 0),
        CRGB(255, 162, 18)
      );
    case CY_EFFECT_FLAME:
      if (energy < 0.24f) return blendRgb(CRGB::Black, CRGB(105, 0, 0), uint8_t(energy * 1060.0f));
      if (energy < 0.58f) return blendRgb(CRGB(105, 0, 0), CRGB(240, 58, 0), uint8_t((energy - 0.24f) * 750.0f));
      if (energy < 0.86f) return blendRgb(CRGB(240, 58, 0), CRGB(255, 170, 20), uint8_t((energy - 0.58f) * 910.0f));
      return blendRgb(CRGB(255, 170, 20), CRGB(255, 226, 92), uint8_t((energy - 0.86f) * 1820.0f));
    case CY_EFFECT_PLASMA_CORE:
      return cyBlend3(h, cyColorOr(0, CRGB(7, 0, 56)), cyColorOr(1, CRGB(0, 174, 255)), cyColorOr(2, CRGB(230, 248, 255)));
    case CY_EFFECT_DEEP_NOISE:
      return cyBlend3(h, cyColorOr(0, CRGB(0, 8, 36)), cyColorOr(1, CRGB(0, 128, 154)), cyColorOr(2, CRGB(72, 230, 190)));
    case CY_EFFECT_AURORA_TUBE:
      return cyBlend3(h, CRGB(0, 12, 40), CRGB(30, 180, 110), CRGB(140, 70, 255));
    case CY_EFFECT_INNER_SWIRL:
      return cyBlend3(h, CRGB(6, 0, 32), CRGB(80, 60, 220), CRGB(0, 230, 200));
    case CY_EFFECT_BUBBLES_VOLUME:
      return cyBlend3(h, CRGB(0, 10, 26), CRGB(0, 110, 170), CRGB(180, 245, 255));
    case CY_EFFECT_RING_RIPPLES:
      return cyBlend3(h, CRGB(0, 8, 28), CRGB(0, 145, 190), CRGB(150, 238, 255));
    case CY_EFFECT_RING_RIPPLES_RAINBOW:
      return CHSV(uint8_t((theta * 40.58451f) + (0.025f * t)), 210, qadd8(12, e8));
    case CY_EFFECT_BOTTOM_RAYS:
      return cyBlend3(h, CRGB(18, 0, 0), CRGB(210, 60, 0), CRGB(255, 186, 42));
    case CY_EFFECT_RISING_BANDS:
      return CHSV(uint8_t((theta * 24.0f) + (0.018f * t)), 220, qadd8(10, h8));
    case CY_EFFECT_HELICAL_PLASMA:
      return cyBlend3(h, CRGB(4, 0, 44), CRGB(150, 35, 230), CRGB(0, 235, 255));
    case CY_EFFECT_NOISE_WAVES_TUBE:
      return cyBlend3(h, CRGB(0, 10, 34), CRGB(0, 120, 170), CRGB(120, 230, 230));
    case CY_EFFECT_CELL_MEMBRANE_FLOW:
      return cyBlend3(h, CRGB(8, 0, 28), CRGB(120, 50, 180), CRGB(80, 240, 210));
    case CY_EFFECT_CROSS_BANDS_TUBE:
      return cyBlend3(h, CRGB(0, 6, 26), CRGB(30, 130, 230), CRGB(255, 120, 60));
  }
  return CRGB::Black;
}

static void outputCyField(SceneContext& context, CyEffectKind kind) {
  const float t = float(strip.now);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t scalar = context.field.blurred[indexOf(x, y, context.surface)];
      const float energy = cyOpticalTransfer(float(scalar) / 255.0f);
      const uint8_t opticalScalar = cyEnergy8(energy);
      const float theta = CY_TWO_PI * (float(x) / float(context.surface.width));
      CRGB color = opticalScalar == 0 ? CRGB::Black : cyColor(kind, energy, theta, t);
      color.nscale8_video(qadd8(4, scale8(opticalScalar, 236)));
      cyLimit(color);
      SEGMENT.setPixelColorXY(x, y, color);
    }
  }
}

static void renderCyAnemone(RenderState&, const Surface& surface, uint16_t) {
  const int W = int(surface.width);
  const int H = int(surface.height);
  const uint16_t step = octopusStep(float(strip.now));
  const float scaleX = octopusScaleFromUi(SEGMENT.intensity);
  const float scaleY = octopusScaleFromUi(SEGMENT.custom1);

  for (uint8_t x = 0; x < surface.width; x++) {
    for (uint8_t y = 0; y < surface.height; y++) {
      const FxCoord coord = fxCoordCylinderShell(x, y, W, H, 0.0f, 0.14f, scaleX, scaleY);
      const OctopusSample sample = octopusSampleFromCoord(coord, W, H);
      SEGMENT.setPixelColorXY(x, y, octopusKernel(sample, step, SEGMENT.custom3));
    }
  }
}

#define CY_DEFINE_SCENE(NAME, KIND) \
  static void build##NAME(SceneContext& context) { buildCyField(context, KIND); } \
  static void output##NAME(SceneContext& context) { outputCyField(context, KIND); } \
  static const SceneDefinition NAME##_SCENE = { build##NAME, cyExactPipelineSettings, output##NAME }; \
  static void render##NAME(RenderState& state, const Surface& surface, uint16_t dt) { renderScene(state, surface, dt, NAME##_SCENE); }

CY_DEFINE_SCENE(CyLavaLamp, CY_EFFECT_LAVA_LAMP)
CY_DEFINE_SCENE(CyFlame, CY_EFFECT_FLAME)
CY_DEFINE_SCENE(CyPlasmaCore, CY_EFFECT_PLASMA_CORE)
CY_DEFINE_SCENE(CyDeepNoise, CY_EFFECT_DEEP_NOISE)
CY_DEFINE_SCENE(CyAuroraTube, CY_EFFECT_AURORA_TUBE)
CY_DEFINE_SCENE(CyInnerSwirl, CY_EFFECT_INNER_SWIRL)
CY_DEFINE_SCENE(CyBubblesVolume, CY_EFFECT_BUBBLES_VOLUME)
CY_DEFINE_SCENE(CyRingRipples, CY_EFFECT_RING_RIPPLES)
CY_DEFINE_SCENE(CyRingRipplesRainbow, CY_EFFECT_RING_RIPPLES_RAINBOW)
CY_DEFINE_SCENE(CyBottomRays, CY_EFFECT_BOTTOM_RAYS)
CY_DEFINE_SCENE(CyRisingBands, CY_EFFECT_RISING_BANDS)
CY_DEFINE_SCENE(CyHelicalPlasma, CY_EFFECT_HELICAL_PLASMA)
CY_DEFINE_SCENE(CyNoiseWavesTube, CY_EFFECT_NOISE_WAVES_TUBE)
CY_DEFINE_SCENE(CyCellMembraneFlow, CY_EFFECT_CELL_MEMBRANE_FLOW)
CY_DEFINE_SCENE(CyCrossBandsTube, CY_EFFECT_CROSS_BANDS_TUBE)

#undef CY_DEFINE_SCENE

} // namespace CylinderLamp
