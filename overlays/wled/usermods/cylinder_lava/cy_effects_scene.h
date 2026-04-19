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

static float cyFieldAnemone(const CyCoord& c, float t) {
  // calm clear liquid in the tube
  const float liquidSlow = cyCylinderNoise(c.theta, c.h, c.r, 1.2f, 1.8f, 1.2f, 0.0f, -0.00008f * t, 0.0f);
  const float liquidDeep = cyCylinderNoise(c.theta, c.h, c.r, 2.0f, 3.0f, 2.0f, 11.0f, -0.00012f * t, 19.0f);
  const float liquid = 0.025f + (0.030f * liquidSlow) + (0.012f * liquidDeep);

  // bottom body / foot: one readable anchored organism base
  const float baseH = cyGauss(c.h, 0.085f, 0.070f);
  const float baseOuter = cyGauss(c.r, 0.80f, 0.15f);
  const float baseInner = cyGauss(c.r, 0.58f, 0.22f);
  const float baseLobes = 0.92f + 0.08f * cosf((4.0f * c.theta) - (0.00022f * t));
  const float base = 1.10f * baseH * ((0.68f * baseOuter) + (0.32f * baseInner)) * baseLobes;

  // crown / collar at the top of the body, from which tentacles grow upward
  const float crownH = cyGauss(c.h, 0.150f, 0.050f);
  const float crownR = cyGauss(c.r, 0.82f, 0.10f);
  const float crown = 0.32f * crownH * crownR * (0.78f + 0.22f * cosf((6.0f * c.theta) - (0.00055f * t)));

  // thick soft tentacles / fringe growing upward
  float tentacles = 0.0f;
  for (uint8_t j = 0; j < 9; j++) {
    const float jf = float(j);
    const float phase = 0.70f * jf;
    const float theta0 = CY_TWO_PI * (jf / 9.0f);

    const float root = 0.125f + (0.010f * sinf(phase));
    const float sway = (0.055f + (0.020f * c.h)) * sinf((2.8f * c.h) - (0.00080f * t) + phase);
    const float curl = 0.045f * sinf((7.0f * c.h) - (0.00105f * t) + (1.3f * phase));
    const float center = theta0 + sway + curl;

    const float dtheta = cyWrappedAngle(c.theta - center);
    const float ang = cyGauss(dtheta, 0.0f, 0.16f);

    const float lift = cySmoothstep(root, root + 0.05f, c.h);
    const float len = 0.25f + (0.05f * sinf(phase));
    const float fall = expf(-(c.h - root) / len);

    const float radOuter = cyGauss(c.r, 0.84f, 0.08f);
    const float radInner = cyGauss(c.r, 0.70f, 0.14f);
    const float rad = (0.74f * radOuter) + (0.26f * radInner);

    const float noise = cyCylinderNoise(c.theta, c.h, c.r, 1.7f, 4.4f, 1.7f, 23.0f * jf, -0.00025f * t, 9.0f * jf);
    const float edge = 0.88f + (0.18f * noise);

    tentacles += 0.52f * ang * lift * fall * rad * edge;
  }

  // slight inner cavity so the base is not a flat glowing lump
  const float cavity = 0.16f * cyGauss(c.h, 0.110f, 0.030f) * cyGauss(c.r, 0.48f, 0.15f);

  return liquid + base + crown + tentacles - cavity;
}

static float cyFieldLavaLamp(const CyCoord& c, float t) {
  const float liquid = 0.08f + 0.06f * cyCylinderNoise(c.theta, c.h, c.r, 1.5f, 2.2f, 1.5f, 0.0f, -0.00012f * t, 0.0f);

  const float thetaC[3] = {0.0f, 2.1f, 4.2f};
  const float hc[3] = {
    0.22f + 0.10f * sinf((0.00033f * t) + 0.0f),
    0.47f + 0.12f * sinf((0.00027f * t) + 1.5f),
    0.71f + 0.10f * sinf((0.00029f * t) + 3.1f)
  };
  const float tc[3] = {
    thetaC[0] + 0.22f * sinf((0.00019f * t) + 0.0f),
    thetaC[1] + 0.18f * sinf((0.00021f * t) + 1.2f),
    thetaC[2] + 0.20f * sinf((0.00017f * t) + 2.4f)
  };
  const float rc[3] = {0.63f, 0.58f, 0.66f};

  float blobs = 0.0f;
  for (uint8_t k = 0; k < 3; k++) {
    const float da = cyWrappedAngle(c.theta - tc[k]) / 0.58f;
    const float dh = (c.h - hc[k]) / 0.18f;
    const float dr = (c.r - rc[k]) / 0.23f;
    blobs += expf(-((da * da) + (dh * dh) + (dr * dr)));
  }

  const float heat = expf(-c.h / 0.20f);
  const float nW = cyCylinderNoise(c.theta, c.h, c.r, 2.0f, 3.6f, 2.0f, 0.0f, -0.00024f * t, 0.0f);
  const float warp = 0.82f + 0.32f * nW;
  const float waxRaw = (blobs * warp) + (0.22f * heat);
  const float wax = cySmoothstep(0.48f, 0.72f, waxRaw);
  return liquid + 1.35f * wax;
}

static float cyFieldFlame(const CyCoord& c, float t) {
  const float sourceA = cyGauss(c.h, 0.04f, 0.05f);
  const float sourceR = cyGauss(c.r, 0.72f, 0.18f);
  const float source = 1.2f * sourceA * sourceR;

  const float thetaT = c.theta + (1.1f * c.h) + (0.0009f * t);
  const float nF1 = cyCylinderNoise(thetaT, c.h, c.r, 2.6f, 5.8f, 2.6f, 0.0f, -0.0014f * t, 0.0f);
  const float nF2 = cyCylinderNoise(thetaT, c.h, c.r, 4.8f, 9.5f, 4.8f, 41.0f, -0.0021f * t, 17.0f);
  const float nF = 0.68f * nF1 + 0.32f * nF2;

  const float envY = expf(-c.h / 0.42f);
  const float envR = cyGauss(c.r, 0.74f, 0.20f);
  const float tongues = cyPow(nF, 1.7f) * envY * envR;
  const float core = 0.75f * expf(-c.h / 0.22f) * cyGauss(c.r, 0.60f, 0.22f);
  return source + 1.25f * tongues + core;
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
  const float nA = cyCylinderNoise(c.theta, c.h, c.r, 1.5f, 4.2f, 1.5f, 0.0f, -0.0004f * t, 0.0f);
  const float upper = 0.75f + 0.25f * c.h;
  const float rad = cyGauss(c.r, 0.68f, 0.24f);
  return cyPow(curtain, 1.5f) * (0.65f + 0.45f * nA) * upper * rad;
}

static float cyFieldInnerSwirl(const CyCoord& c, float t) {
  const float swirl = 0.5f + 0.5f * sinf((4.2f * c.theta) + (7.0f * c.h) - (0.0010f * t));
  const float body = cyGauss(c.r, 0.58f, 0.24f);
  return swirl * body;
}

static float cyFieldBubblesVolume(const CyCoord& c, float t) {
  const float nB1 = cyCylinderNoise(c.theta, c.h, c.r, 2.2f, 3.8f, 2.2f, 0.0f, -0.0003f * t, 0.0f);
  const float nB2 = cyCylinderNoise(c.theta, c.h, c.r, 4.4f, 7.6f, 4.4f, 71.0f, -0.0004f * t, 29.0f);
  const float cell = fabsf(nB1 - nB2);
  const float body = cyGauss(c.r, 0.66f, 0.24f);
  return cyPow(cell, 1.35f) * body;
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
  const float nRB = cyCylinderNoise(c.theta, c.h, c.r, 1.8f, 5.0f, 1.8f, 0.0f, -0.0011f * t, 0.0f);
  const float rad = cyGauss(c.r, 0.74f, 0.18f);
  return cyPow(bands, 1.3f) * (0.75f + 0.35f * nRB) * rad;
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
  const float nC1 = cyCylinderNoise(c.theta, c.h, c.r, 2.4f, 4.4f, 2.4f, 0.0f, -0.0004f * t, 0.0f);
  const float nC2 = cyCylinderNoise(c.theta, c.h, c.r, 4.8f, 8.8f, 4.8f, 19.0f, -0.0006f * t, 83.0f);
  const float mem = 1.0f - fabsf(nC1 - nC2);
  const float body = cyGauss(c.r, 0.66f, 0.24f);
  return cyPow(mem, 1.8f) * body;
}

static float cyFieldCrossBandsTube(const CyCoord& c, float t) {
  const float c1 = 0.5f + 0.5f * sinf((6.0f * c.theta) + (8.0f * c.h) - (0.0011f * t));
  const float c2 = 0.5f + 0.5f * sinf((-4.0f * c.theta) + (7.0f * c.h) + (0.0009f * t));
  const float body = cyGauss(c.r, 0.70f, 0.22f);
  return (c1 * c2) * body;
}

static float cyField(CyEffectKind kind, const CyCoord& c, float t) {
  switch (kind) {
    case CY_EFFECT_ANEMONE: return cyFieldAnemone(c, t);
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
  const float totalWeight = cyDepthWeight(0) + cyDepthWeight(1) + cyDepthWeight(2) + cyDepthWeight(3);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    for (uint8_t x = 0; x < context.surface.width; x++) {
      float accumulated = 0.0f;

      for (uint8_t sample = 0; sample < CY_DEPTH_SAMPLES; sample++) {
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
        cyColorOr(0, CRGB(2, 10, 28)),
        cyColorOr(1, CRGB(86, 56, 196)),
        cyColorOr(2, CRGB(150, 238, 255))
      );
    case CY_EFFECT_LAVA_LAMP:
      return cyBlend3(h, cyColorOr(0, CRGB(12, 0, 0)), cyColorOr(1, CRGB(218, 52, 0)), cyColorOr(2, CRGB(255, 136, 8)));
    case CY_EFFECT_FLAME:
      if (energy < 0.28f) return blendRgb(CRGB::Black, CRGB(90, 0, 0), uint8_t(energy * 910.0f));
      if (energy < 0.62f) return blendRgb(CRGB(90, 0, 0), CRGB(230, 64, 0), uint8_t((energy - 0.28f) * 750.0f));
      if (energy < 0.88f) return blendRgb(CRGB(230, 64, 0), CRGB(255, 178, 24), uint8_t((energy - 0.62f) * 980.0f));
      return blendRgb(CRGB(255, 178, 24), CRGB(255, 232, 150), uint8_t((energy - 0.88f) * 2125.0f));
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
      const float energy = float(scalar) / 255.0f;
      const float theta = CY_TWO_PI * (float(x) / float(context.surface.width));
      CRGB color = cyColor(kind, energy, theta, t);
      color.nscale8_video(qadd8(4, scale8(scalar, 236)));
      cyLimit(color);
      SEGMENT.setPixelColorXY(x, y, color);
    }
  }
}

#define CY_DEFINE_SCENE(NAME, KIND) \
  static void build##NAME(SceneContext& context) { buildCyField(context, KIND); } \
  static void output##NAME(SceneContext& context) { outputCyField(context, KIND); } \
  static const SceneDefinition NAME##_SCENE = { build##NAME, cyExactPipelineSettings, output##NAME }; \
  static void render##NAME(RenderState& state, const Surface& surface, uint16_t dt) { renderScene(state, surface, dt, NAME##_SCENE); }

CY_DEFINE_SCENE(CyAnemone, CY_EFFECT_ANEMONE)
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
