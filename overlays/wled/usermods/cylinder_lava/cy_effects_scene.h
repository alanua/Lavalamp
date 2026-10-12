#pragma once

#include "cylinder_volume.h"

namespace CylinderLamp {

enum CyEffectKind : uint8_t {
  CY_EFFECT_TIDAL_BLOOM = 0,
  CY_EFFECT_FLAME,
  CY_EFFECT_LAVA_FLOW,
  CY_EFFECT_OCEAN_DRIFT,
  CY_EFFECT_AURORA,
  CY_EFFECT_NEBULA,
  CY_EFFECT_ENERGY_PULSE,
  CY_EFFECT_BUBBLES,
  CY_EFFECT_PARTICLE_STORM,
  CY_EFFECT_RAINBOW_SPIRAL,
  CY_EFFECT_MATRIX_RAIN,
  CY_EFFECT_ELECTRIC_STORM,
  CY_EFFECT_HEARTBEAT,
  CY_EFFECT_DREAMSCAPE,
  CY_EFFECT_ANEMONE,
  CY_EFFECT_RIPPLE_RINGS,
  CY_EFFECT_COMET_TRAILS
};

static PipelineSettings cySmoothPipelineSettings(const SceneContext&) {
  PipelineSettings settings;
  settings.temporalAlpha = 92;
  settings.blurX = 42;
  settings.blurY = 30;
  settings.floor = 4;
  settings.ceiling = 255;
  settings.contrast = 138;
  return settings;
}

static inline uint8_t cyRuntimeDepthSamples(CyEffectKind kind) {
  switch (kind) {
    case CY_EFFECT_NEBULA:
    case CY_EFFECT_BUBBLES:
    case CY_EFFECT_PARTICLE_STORM:
      return 2;
    default:
      return 1;
  }
}

static inline float cyOpticalTransfer(float energy) {
  energy = cyClamp((energy - 0.055f) / 0.945f, 0.0f, 1.0f);
  energy = cyPow(energy, 0.78f);
  return cyClamp(energy * 1.12f, 0.0f, 1.0f);
}

static inline float cyAngleGauss(float theta, float center, float width) {
  return cyGauss(cyWrappedAngle(theta - center), 0.0f, width);
}

static inline float cyPulse01(float phase) {
  return 0.5f + 0.5f * sinf(phase);
}

static float cyFieldTidalBloom(const CyCoord& c, float t) {
  float blooms = 0.0f;
  for (uint8_t i = 0; i < 4; i++) {
    const float fi = float(i);
    const float centerTheta =
        (CY_TWO_PI * fi * 0.25f) +
        (0.19f * sinf((0.00024f * t) + (1.5f * fi)));
    const float centerH =
        0.18f +
        (0.64f * cyFract((0.000045f * t) + (0.23f * fi)));
    const float petal =
        cyAngleGauss(c.theta, centerTheta, 0.42f) *
        cyGauss(c.h, centerH, 0.14f) *
        cyGauss(c.r, 0.72f, 0.24f);
    blooms += petal;
  }
  const float tide = 0.5f + 0.5f * sinf((2.0f * c.theta) - (5.0f * c.h) + (0.00048f * t));
  const float floorFeed = 0.30f * expf(-c.h / 0.18f) * cyGauss(c.r, 0.74f, 0.26f);
  return (0.90f * blooms) + (0.26f * tide * cyGauss(c.r, 0.60f, 0.32f)) + floorFeed;
}

static float cyFieldFlame(const CyCoord& c, float t) {
  const float sway =
      0.22f * sinf((3.0f * c.h) - (0.00115f * t)) +
      0.07f * sinf((8.0f * c.h) - (0.0019f * t));
  const float core =
      cyAngleGauss(c.theta, sway, 0.30f + (0.22f * c.h)) *
      cyGauss(c.r, 0.78f, 0.23f) *
      expf(-c.h / 0.52f);
  const float tongues =
      cyPow(cyPulse01((7.0f * c.theta) + (9.0f * c.h) - (0.0016f * t)), 2.6f) *
      cyGauss(c.r, 0.84f, 0.18f) *
      expf(-c.h / 0.67f);
  const float source = 1.25f * expf(-c.h / 0.10f) * cyGauss(c.r, 0.76f, 0.28f);
  return source + (1.05f * core) + (0.38f * tongues);
}

static float cyFieldLavaFlow(const CyCoord& c, float t) {
  const float reservoir =
      1.18f * expf(-c.h / 0.10f) * cyGauss(c.r, 0.72f, 0.28f);
  float blobs = 0.0f;
  for (uint8_t i = 0; i < 4; i++) {
    const float fi = float(i);
    const float centerTheta =
        (1.47f * fi) + (0.24f * sinf((0.00015f * t) + (1.3f * fi)));
    const float centerH =
        0.16f + (0.70f * cyFract((0.000036f * t) + (0.27f * fi)));
    blobs +=
        cyAngleGauss(c.theta, centerTheta, 0.38f + (0.04f * fi)) *
        cyGauss(c.h, centerH, 0.13f + (0.015f * fi)) *
        cyGauss(c.r, 0.66f, 0.25f);
  }
  const float bridge =
      0.24f * cyPulse01((2.1f * c.theta) - (3.8f * c.h) + (0.00025f * t)) *
      cyGauss(c.r, 0.64f, 0.30f);
  return reservoir + (1.02f * blobs) + bridge;
}

static float cyFieldOceanDrift(const CyCoord& c, float t) {
  const float waveA = cyPulse01((2.2f * c.theta) + (7.0f * c.h) - (0.00042f * t));
  const float waveB = cyPulse01((-1.3f * c.theta) + (4.2f * c.h) + (0.00029f * t));
  const float swell = 0.58f * waveA + 0.42f * waveB;
  const float driftNoise =
      cyCylinderNoise(c.theta, c.h, c.r, 1.7f, 3.2f, 1.7f, 0.0f, -0.00018f * t, 0.0f);
  const float shell = cyGauss(c.r, 0.74f, 0.28f);
  return (0.20f + (0.64f * swell) + (0.28f * driftNoise)) * shell;
}

static float cyFieldAurora(const CyCoord& c, float t) {
  const float ribbonCenter =
      0.56f +
      (0.16f * sinf((1.8f * c.theta) - (0.00031f * t))) +
      (0.09f * sinf((4.4f * c.theta) + (0.00019f * t)));
  const float curtain = cyGauss(c.h, ribbonCenter, 0.16f);
  const float folds = 0.48f + 0.52f * cyPulse01((5.0f * c.theta) + (3.0f * c.h) - (0.00034f * t));
  const float shell = cyGauss(c.r, 0.78f, 0.22f);
  return (0.18f + (1.02f * curtain * folds)) * shell;
}

static float cyFieldNebula(const CyCoord& c, float t) {
  const float n1 =
      cyCylinderNoise(c.theta, c.h, c.r, 1.4f, 2.5f, 1.4f, 0.0f, -0.00010f * t, 0.0f);
  const float n2 =
      cyCylinderNoise(c.theta + 1.7f, c.h, c.r, 3.2f, 4.1f, 3.2f, 0.0f, 0.00007f * t, 0.0f);
  const float cloud = cyPow(cyClamp((0.68f * n1) + (0.42f * n2) - 0.25f, 0.0f, 1.0f), 1.55f);
  const float core = cyGauss(c.h, 0.54f + (0.08f * sinf(0.00015f * t)), 0.34f);
  return (0.10f + (1.12f * cloud)) * core * cyGauss(c.r, 0.58f, 0.40f);
}

static float cyFieldEnergyPulse(const CyCoord& c, float t) {
  const float phase = cyFract(0.00016f * t);
  const float ringH = 0.08f + (0.84f * phase);
  const float ring = cyGauss(c.h, ringH, 0.060f);
  const float corePulse = cyPow(cyPulse01(0.00115f * t), 3.5f);
  const float core = corePulse * cyGauss(c.h, 0.50f, 0.22f) * cyGauss(c.r, 0.34f, 0.20f);
  const float shell = cyGauss(c.r, 0.72f, 0.24f);
  return (0.18f + (1.10f * ring)) * shell + (0.85f * core);
}

static float cyFieldBubbles(const CyCoord& c, float t) {
  float bubbles = 0.0f;
  for (uint8_t i = 0; i < 6; i++) {
    const float fi = float(i);
    const float centerTheta =
        (CY_TWO_PI * fi / 6.0f) +
        (0.21f * sinf((0.00019f * t) + (1.1f * fi)));
    const float centerH = cyFract((0.000052f * t) + (0.17f * fi));
    const float shellH = fabsf(c.h - centerH);
    const float shell =
        cyAngleGauss(c.theta, centerTheta, 0.24f) *
        cyGauss(shellH, 0.075f, 0.040f) *
        cyGauss(c.r, 0.76f, 0.20f);
    bubbles += shell;
  }
  return 0.06f + (1.25f * bubbles);
}

static float cyFieldParticleStorm(const CyCoord& c, float t) {
  float particles = 0.0f;
  for (uint8_t i = 0; i < 8; i++) {
    const float fi = float(i);
    const float centerTheta =
        cyFract((0.000021f * t * (1.0f + 0.07f * fi)) + (0.137f * fi)) * CY_TWO_PI;
    const float centerH =
        cyFract((0.000047f * t * (1.0f + 0.04f * fi)) + (0.211f * fi));
    particles +=
        cyAngleGauss(c.theta, centerTheta, 0.105f) *
        cyGauss(c.h, centerH, 0.060f) *
        cyGauss(c.r, 0.80f, 0.18f);
  }
  const float wind =
      0.18f * cyPow(cyPulse01((9.0f * c.theta) + (5.0f * c.h) - (0.0010f * t)), 3.0f) *
      cyGauss(c.r, 0.76f, 0.24f);
  return (1.40f * particles) + wind;
}

static float cyFieldRainbowSpiral(const CyCoord& c, float t) {
  const float helixA = cyPow(cyPulse01((5.0f * c.theta) + (8.0f * c.h) - (0.00078f * t)), 2.2f);
  const float helixB = cyPow(cyPulse01((-3.0f * c.theta) + (5.5f * c.h) + (0.00052f * t)), 3.0f);
  return (0.12f + (0.74f * helixA) + (0.42f * helixB)) * cyGauss(c.r, 0.72f, 0.24f);
}

static float cyFieldMatrixRain(const CyCoord& c, float t) {
  const float column = floorf((c.theta / CY_TWO_PI) * 12.0f);
  const float phase = cyFract((0.381966f * column) + (0.00011f * t));
  const float head = 1.0f - phase;
  float trail = c.h - head;
  if (trail < 0.0f) trail += 1.0f;
  const float headGlow = cyGauss(trail, 0.0f, 0.045f);
  const float tail = trail < 0.38f ? expf(-trail / 0.12f) : 0.0f;
  const float lane =
      0.28f + 0.72f * cyPow(cyPulse01((12.0f * c.theta) + (0.00023f * t)), 4.0f);
  return (0.18f + headGlow + (0.58f * tail)) * lane * cyGauss(c.r, 0.80f, 0.18f);
}

static float cyFieldElectricStorm(const CyCoord& c, float t) {
  const float arcA = fabsf(sinf((4.5f * c.theta) + (8.0f * c.h) - (0.00091f * t)));
  const float arcB = fabsf(sinf((-6.0f * c.theta) + (5.0f * c.h) + (0.00067f * t)));
  const float branch = cyPow(cyClamp(1.0f - (2.0f * fabsf(arcA - arcB)), 0.0f, 1.0f), 4.0f);
  const float flash = 0.64f + (0.36f * cyPow(cyPulse01(0.00075f * t), 2.0f));
  return (0.10f + (1.15f * branch * flash)) * cyGauss(c.r, 0.73f, 0.26f);
}

static float cyFieldHeartbeat(const CyCoord& c, float t) {
  const float phase = fmodf(0.0012f * t, CY_TWO_PI);
  const float beatA = cyGauss(sinf(phase), 0.92f, 0.18f);
  const float beatB = cyGauss(sinf(phase + 0.72f), 0.92f, 0.20f);
  const float beat = cyClamp(beatA + (0.72f * beatB), 0.0f, 1.4f);
  const float ring = cyGauss(c.h, 0.48f + (0.08f * beat), 0.075f + (0.025f * beat));
  const float core = cyGauss(c.r, 0.58f, 0.30f);
  return (0.12f + (1.15f * beat * ring)) * core;
}

static float cyFieldDreamscape(const CyCoord& c, float t) {
  const float a = cyPulse01((1.7f * c.theta) + (3.2f * c.h) - (0.00021f * t));
  const float b = cyPulse01((-2.3f * c.theta) + (2.6f * c.h) + (0.00016f * t));
  const float n =
      cyCylinderNoise(c.theta, c.h, c.r, 1.1f, 2.1f, 1.1f, 0.0f, 0.00008f * t, 0.0f);
  const float haze = 0.24f + (0.46f * a) + (0.34f * b) + (0.24f * n);
  return haze * cyGauss(c.r, 0.61f, 0.40f);
}

static float cyFieldAnemone(const CyCoord& c, float t) {
  const float sway = 0.18f * sinf((2.0f * c.h) - (0.00031f * t));
  const float tentacles =
      cyPow(cyPulse01((8.0f * (c.theta - sway)) + (6.0f * c.h) - (0.00043f * t)), 4.0f);
  const float body = cyGauss(c.h, 0.20f, 0.18f) * cyGauss(c.r, 0.66f, 0.30f);
  const float arms = tentacles * expf(-c.h / 0.62f) * cyGauss(c.r, 0.76f, 0.22f);
  return (0.82f * body) + (0.88f * arms);
}

static float cyFieldRippleRings(const CyCoord& c, float t) {
  const float p1 = cyFract(0.00010f * t);
  const float p2 = cyFract(0.46f + (0.000075f * t));
  const float ringA = cyGauss(fabsf(c.h - p1), 0.0f, 0.047f);
  const float ringB = cyGauss(fabsf(c.h - p2), 0.0f, 0.065f);
  const float wobble = 0.82f + 0.18f * sinf((3.0f * c.theta) + (0.00024f * t));
  return ((0.90f * ringA) + (0.68f * ringB)) * wobble * cyGauss(c.r, 0.74f, 0.22f);
}

static float cyFieldCometTrails(const CyCoord& c, float t) {
  float comets = 0.0f;
  for (uint8_t i = 0; i < 3; i++) {
    const float fi = float(i);
    const float headTheta = cyFract((0.000047f * t) + (0.333f * fi)) * CY_TWO_PI;
    const float headH = 0.24f + (0.25f * fi) + (0.07f * sinf((0.00019f * t) + fi));
    float d = cyWrappedAngle(c.theta - headTheta);
    if (d > 0.0f) d -= CY_TWO_PI;
    const float head =
        cyGauss(d, 0.0f, 0.16f) *
        cyGauss(c.h, headH, 0.075f);
    const float trail =
        (d <= 0.0f && d > -1.65f)
          ? expf(d / 0.55f) * cyGauss(c.h, headH + (0.035f * sinf(5.0f * d)), 0.095f)
          : 0.0f;
    comets += (1.18f * head) + (0.48f * trail);
  }
  return comets * cyGauss(c.r, 0.80f, 0.18f);
}

static float cyField(CyEffectKind kind, const CyCoord& c, float t) {
  switch (kind) {
    case CY_EFFECT_TIDAL_BLOOM: return cyFieldTidalBloom(c, t);
    case CY_EFFECT_FLAME: return cyFieldFlame(c, t);
    case CY_EFFECT_LAVA_FLOW: return cyFieldLavaFlow(c, t);
    case CY_EFFECT_OCEAN_DRIFT: return cyFieldOceanDrift(c, t);
    case CY_EFFECT_AURORA: return cyFieldAurora(c, t);
    case CY_EFFECT_NEBULA: return cyFieldNebula(c, t);
    case CY_EFFECT_ENERGY_PULSE: return cyFieldEnergyPulse(c, t);
    case CY_EFFECT_BUBBLES: return cyFieldBubbles(c, t);
    case CY_EFFECT_PARTICLE_STORM: return cyFieldParticleStorm(c, t);
    case CY_EFFECT_RAINBOW_SPIRAL: return cyFieldRainbowSpiral(c, t);
    case CY_EFFECT_MATRIX_RAIN: return cyFieldMatrixRain(c, t);
    case CY_EFFECT_ELECTRIC_STORM: return cyFieldElectricStorm(c, t);
    case CY_EFFECT_HEARTBEAT: return cyFieldHeartbeat(c, t);
    case CY_EFFECT_DREAMSCAPE: return cyFieldDreamscape(c, t);
    case CY_EFFECT_ANEMONE: return cyFieldAnemone(c, t);
    case CY_EFFECT_RIPPLE_RINGS: return cyFieldRippleRings(c, t);
    case CY_EFFECT_COMET_TRAILS: return cyFieldCometTrails(c, t);
  }
  return 0.0f;
}

static void buildCyField(SceneContext& context, CyEffectKind kind) {
  advanceMotion(context.motion, context.dt, MotionRates());
  const float t = float(strip.now);
  const uint8_t depthSamples = cyRuntimeDepthSamples(kind);
  float totalWeight = 0.0f;
  for (uint8_t sample = 0; sample < depthSamples; sample++) {
    totalWeight += cyDepthWeight(sample);
  }

  for (uint8_t y = 0; y < context.surface.height; y++) {
    for (uint8_t x = 0; x < context.surface.width; x++) {
      float accumulated = 0.0f;
      for (uint8_t sample = 0; sample < depthSamples; sample++) {
        const CyCoord c = cyCoord(x, y, sample, context.surface);
        accumulated += cyDepthWeight(sample) * cyField(kind, c, t);
      }
      context.field.raw[indexOf(x, y, context.surface)] =
          cyEnergy8(accumulated / totalWeight);
    }
  }
}

static CRGB cyColor(CyEffectKind kind, float energy, float theta, float hCoord, float t) {
  const float h = cyHighlight(energy);
  const uint8_t value = qadd8(10, scale8(cyEnergy8(energy), 245));

  switch (kind) {
    case CY_EFFECT_TIDAL_BLOOM:
      return cyBlend3(h, CRGB(0, 8, 28), CRGB(0, 145, 170), CRGB(255, 102, 205));
    case CY_EFFECT_FLAME:
      return cyBlend3(h, CRGB(30, 0, 0), CRGB(238, 50, 0), CRGB(255, 220, 70));
    case CY_EFFECT_LAVA_FLOW:
      return cyBlend3(h, CRGB(12, 0, 0), CRGB(205, 32, 0), CRGB(255, 145, 15));
    case CY_EFFECT_OCEAN_DRIFT:
      return cyBlend3(h, CRGB(0, 6, 30), CRGB(0, 92, 166), CRGB(84, 230, 240));
    case CY_EFFECT_AURORA:
      return cyBlend3(h, CRGB(0, 8, 36), CRGB(20, 205, 118), CRGB(160, 75, 255));
    case CY_EFFECT_NEBULA:
      return cyBlend3(h, CRGB(8, 0, 28), CRGB(105, 30, 170), CRGB(240, 105, 210));
    case CY_EFFECT_ENERGY_PULSE:
      return cyBlend3(h, CRGB(0, 4, 34), CRGB(0, 175, 245), CRGB(226, 250, 255));
    case CY_EFFECT_BUBBLES:
      return cyBlend3(h, CRGB(0, 9, 28), CRGB(0, 120, 184), CRGB(190, 248, 255));
    case CY_EFFECT_PARTICLE_STORM:
      return cyBlend3(h, CRGB(10, 0, 28), CRGB(190, 44, 150), CRGB(255, 224, 120));
    case CY_EFFECT_RAINBOW_SPIRAL:
      return CHSV(uint8_t((theta * 40.58451f) + (hCoord * 96.0f) + (0.018f * t)), 232, value);
    case CY_EFFECT_MATRIX_RAIN:
      return cyBlend3(h, CRGB(0, 10, 0), CRGB(0, 155, 28), CRGB(190, 255, 205));
    case CY_EFFECT_ELECTRIC_STORM:
      return cyBlend3(h, CRGB(2, 0, 32), CRGB(90, 70, 245), CRGB(225, 245, 255));
    case CY_EFFECT_HEARTBEAT:
      return cyBlend3(h, CRGB(18, 0, 5), CRGB(205, 8, 44), CRGB(255, 165, 184));
    case CY_EFFECT_DREAMSCAPE:
      return cyBlend3(h, CRGB(8, 0, 34), CRGB(60, 70, 210), CRGB(70, 235, 205));
    case CY_EFFECT_ANEMONE:
      return cyBlend3(h, CRGB(0, 7, 24), CRGB(110, 20, 190), CRGB(105, 245, 255));
    case CY_EFFECT_RIPPLE_RINGS:
      return cyBlend3(h, CRGB(0, 8, 30), CRGB(0, 152, 200), CRGB(170, 242, 255));
    case CY_EFFECT_COMET_TRAILS:
      return cyBlend3(h, CRGB(4, 0, 25), CRGB(70, 80, 225), CRGB(255, 238, 175));
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
      const float theta = CY_TWO_PI * ((float(x) + 0.5f) / float(context.surface.width));
      const float hCoord =
          context.surface.height <= 1
            ? 0.0f
            : float(context.surface.height - 1 - y) / float(context.surface.height - 1);
      CRGB color =
          opticalScalar == 0 ? CRGB::Black : cyColor(kind, energy, theta, hCoord, t);
      color.nscale8_video(qadd8(4, scale8(opticalScalar, 236)));
      cyLimit(color);
      SEGMENT.setPixelColorXY(x, y, color);
    }
  }
}

#define CY_DEFINE_SCENE(NAME, KIND) \
  static void build##NAME(SceneContext& context) { buildCyField(context, KIND); } \
  static void output##NAME(SceneContext& context) { outputCyField(context, KIND); } \
  static const SceneDefinition NAME##_SCENE = { build##NAME, cySmoothPipelineSettings, output##NAME }; \
  static void render##NAME(RenderState& state, const Surface& surface, uint16_t dt) { renderScene(state, surface, dt, NAME##_SCENE); }

CY_DEFINE_SCENE(CyTidalBloom, CY_EFFECT_TIDAL_BLOOM)
CY_DEFINE_SCENE(CyFlame, CY_EFFECT_FLAME)
CY_DEFINE_SCENE(CyLavaFlow, CY_EFFECT_LAVA_FLOW)
CY_DEFINE_SCENE(CyOceanDrift, CY_EFFECT_OCEAN_DRIFT)
CY_DEFINE_SCENE(CyAurora, CY_EFFECT_AURORA)
CY_DEFINE_SCENE(CyNebula, CY_EFFECT_NEBULA)
CY_DEFINE_SCENE(CyEnergyPulse, CY_EFFECT_ENERGY_PULSE)
CY_DEFINE_SCENE(CyBubbles, CY_EFFECT_BUBBLES)
CY_DEFINE_SCENE(CyParticleStorm, CY_EFFECT_PARTICLE_STORM)
CY_DEFINE_SCENE(CyRainbowSpiral, CY_EFFECT_RAINBOW_SPIRAL)
CY_DEFINE_SCENE(CyMatrixRain, CY_EFFECT_MATRIX_RAIN)
CY_DEFINE_SCENE(CyElectricStorm, CY_EFFECT_ELECTRIC_STORM)
CY_DEFINE_SCENE(CyHeartbeat, CY_EFFECT_HEARTBEAT)
CY_DEFINE_SCENE(CyDreamscape, CY_EFFECT_DREAMSCAPE)
CY_DEFINE_SCENE(CyAnemone, CY_EFFECT_ANEMONE)
CY_DEFINE_SCENE(CyRippleRings, CY_EFFECT_RIPPLE_RINGS)
CY_DEFINE_SCENE(CyCometTrails, CY_EFFECT_COMET_TRAILS)

#undef CY_DEFINE_SCENE

} // namespace CylinderLamp
