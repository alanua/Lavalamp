#pragma once

#include "cylinder_scene_contract.h"

namespace CylinderLamp {

struct Lobe {
  uint16_t x = 0;      // Q8 angular position around the cylinder
  uint16_t y = 0;      // Q8 logical row position
  uint16_t radius = 0; // Q8 pixel radius
  uint8_t weight = 0;
};

struct LavaMassSpec {
  uint8_t riseOffset = 0;
  uint8_t angleOffset = 0;
  uint8_t size = 0;
  uint8_t weight = 0;

  LavaMassSpec() {}
  LavaMassSpec(uint8_t rise, uint8_t angle, uint8_t massSize, uint8_t massWeight)
    : riseOffset(rise),
      angleOffset(angle),
      size(massSize),
      weight(massWeight) {
  }
};

static const uint8_t LAVA_PRIMARY_MASS_COUNT = 3;

static inline CRGB lavaPalette(uint8_t value, const CRGB& deep, const CRGB& glow, const CRGB& core) {
  const CRGB shadow = blendRgb(deep, CRGB(34, 2, 0), 72);
  const CRGB ember = blendRgb(deep, glow, 54);
  const CRGB molten = blendRgb(glow, core, 76);

  if (value < 84) return blendRgb(deep, shadow, uint8_t((uint16_t(value) * 255U) / 84U));
  if (value < 154) return blendRgb(shadow, ember, uint8_t((uint16_t(value - 84) * 255U) / 70U));
  if (value < 214) return blendRgb(ember, glow, uint8_t((uint16_t(value - 154) * 255U) / 60U));
  if (value < 242) return blendRgb(glow, molten, uint8_t((uint16_t(value - 214) * 255U) / 28U));
  return blendRgb(molten, core, uint8_t((uint16_t(value - 242) * 255U) / 13U));
}

static inline uint8_t lavaLobeFalloff(uint8_t x, uint8_t y, const Surface& surface, const Lobe& lobe, uint8_t yStretch) {
  return radialFalloffQ8(x, y, surface, lobe.x, lobe.y, lobe.radius, yStretch);
}

static inline uint16_t wrapCenterXQ8(int32_t x, const Surface& surface) {
  const int32_t circumference = int32_t(surface.width) << 8;
  if (circumference <= 0) return 0;
  while (x < 0) x += circumference;
  while (x >= circumference) x -= circumference;
  return uint16_t(x);
}

static inline uint16_t clampCenterYQ8(int32_t y, const Surface& surface) {
  const int32_t bottom = int32_t(surface.height - 1) << 8;
  if (y < 0) return 0;
  if (y > bottom) return uint16_t(bottom);
  return uint16_t(y);
}

static inline CRGB lavaColorOr(uint8_t slot, const CRGB& fallback, uint16_t maxTotal, uint8_t maxGreen, uint8_t maxBlue, bool rejectBright) {
  CRGB color = CRGB(SEGCOLOR(slot));
  uint16_t total = uint16_t(color.r) + color.g + color.b;
  if (total <= 4 || (rejectBright && total > maxTotal)) return fallback;

  if (color.g > maxGreen) color.g = maxGreen;
  if (color.b > maxBlue) color.b = maxBlue;
  if (color.r < color.g) color.r = qadd8(color.r, scale8(color.g - color.r, 96));

  total = uint16_t(color.r) + color.g + color.b;
  if (total > maxTotal) {
    color.nscale8_video(uint8_t((uint32_t(maxTotal) * 255U) / total));
  }
  return color;
}

static inline void limitLavaColor(CRGB& color) {
  limitColor(color, 238, 158, 18, 345);
}

static MotionRates lavaMotionRates(const SceneControls& controls) {
  MotionRates rates;
  rates.angular = 4 + scale8(controls.flow, 18);
  rates.rise = 3 + scale8(controls.flow, 12);
  rates.rotation = 2 + scale8(controls.heat, 8);
  rates.deform = 2 + scale8(controls.softness, 10);
  return rates;
}

static PipelineSettings lavaPipelineSettings(const SceneContext& context) {
  PipelineSettings settings;
  settings.temporalAlpha = 198 + scale8(context.controls.viscosity, 42);
  settings.blurX = 30 + scale8(context.controls.softness, 42);
  settings.blurY = 28 + scale8(context.controls.softness, 34);
  settings.floor = 8;
  settings.ceiling = 238;
  settings.contrast = 184;
  return settings;
}

static inline uint8_t massProgress8(const MotionState& motion, const LavaMassSpec& spec) {
  return phase8(motion.risePhase + (uint16_t(spec.riseOffset) << 8));
}

static inline uint16_t massYQ8(const SceneContext& context, uint8_t progress) {
  const uint8_t lifted = scale8(ease8InOutApprox(progress), 218);
  return uint16_t(255U - lifted) * (context.surface.height - 1);
}

static inline uint16_t massXQ8(const SceneContext& context, const LavaMassSpec& spec, uint8_t progress) {
  const uint16_t base = orbitXQ8(context.surface, context.motion, spec.angleOffset, 10);
  const int16_t slowLean = int16_t(sin8_t(progress + spec.angleOffset)) - 128;
  return wrapCenterXQ8(int32_t(base) + slowLean * 2, context.surface);
}

static Lobe buildPrimaryMass(const SceneContext& context, const LavaMassSpec& spec) {
  const uint8_t progress = massProgress8(context.motion, spec);
  const uint8_t breathe = sin8_t(phase8(context.motion.deformPhase) + spec.angleOffset);
  const uint16_t baseRadius = 1060 + uint16_t(context.controls.size) * 3U;
  const uint16_t breathRadius = uint16_t(scale8(breathe, 90)) * 2U;
  const uint8_t lowerPresence = 255 - scale8(progress, 66);

  Lobe mass;
  mass.x = massXQ8(context, spec, progress);
  mass.y = massYQ8(context, progress);
  mass.radius = baseRadius + uint16_t(spec.size) * 4U + breathRadius;
  mass.weight = scale8(spec.weight, lowerPresence);
  return mass;
}

static Lobe buildShoulderMass(const SceneContext& context, const LavaMassSpec& spec, const Lobe& primary) {
  const uint8_t phase = phase8(context.motion.deformPhase) + spec.angleOffset;
  const int16_t dx = (int16_t(sin8_t(phase)) - 128) * 3;
  const int16_t dy = (int16_t(cos8_t(phase + 41)) - 128) * 2;

  Lobe shoulder;
  shoulder.x = wrapCenterXQ8(int32_t(primary.x) + dx, context.surface);
  shoulder.y = clampCenterYQ8(int32_t(primary.y) + dy, context.surface);
  shoulder.radius = uint16_t((uint32_t(primary.radius) * 172U) >> 8);
  shoulder.weight = scale8(primary.weight, 78);
  return shoulder;
}

static void buildLavaField(SceneContext& context) {
  advanceMotion(context.motion, context.dt, lavaMotionRates(context.controls));

  static const LavaMassSpec masses[LAVA_PRIMARY_MASS_COUNT] = {
    { 18,  25, 46, 178 },
    { 104, 142, 28, 156 },
    { 191, 219, 38, 168 }
  };
  Lobe primary[LAVA_PRIMARY_MASS_COUNT];
  Lobe shoulder[LAVA_PRIMARY_MASS_COUNT];

  for (uint8_t i = 0; i < LAVA_PRIMARY_MASS_COUNT; i++) {
    primary[i] = buildPrimaryMass(context, masses[i]);
    shoulder[i] = buildShoulderMass(context, masses[i], primary[i]);
  }

  const uint8_t heatBias = 32 + scale8(context.controls.heat, 92);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    const uint8_t h = heightFromBottom8(y, context.surface.height);
    const uint8_t bottomHeat = scale8(255 - h, heatBias);

    for (uint8_t x = 0; x < context.surface.width; x++) {
      uint8_t field = scale8(bottomHeat, 44);

      for (uint8_t i = 0; i < LAVA_PRIMARY_MASS_COUNT; i++) {
        addLayer(field, lavaLobeFalloff(x, y, context.surface, primary[i], 88), primary[i].weight);
        addLayer(field, lavaLobeFalloff(x, y, context.surface, shoulder[i], 96), shoulder[i].weight);
      }

      context.field.raw[indexOf(x, y, context.surface)] = field;
    }
  }
}

static void outputLava(SceneContext& context) {
  const CRGB deep = lavaColorOr(0, CRGB(3, 0, 1), 56, 10, 4, true);
  const CRGB glow = lavaColorOr(1, CRGB(224, 72, 0), 310, 104, 10, false);
  const CRGB core = lavaColorOr(2, CRGB(255, 138, 8), 390, 158, 18, false);

  for (uint8_t y = 0; y < context.surface.height; y++) {
    const uint8_t h = heightFromBottom8(y, context.surface.height);
    const uint8_t bottomGlow = scale8(255 - h, 14);

    for (uint8_t x = 0; x < context.surface.width; x++) {
      const uint8_t scalar = qadd8(context.field.blurred[indexOf(x, y, context.surface)], bottomGlow);
      const uint8_t safeScalar = scalar > 228 ? 228 : scalar;
      const uint8_t depthScalar = qsub8(safeScalar, 14);
      CRGB color = lavaPalette(ease8InOutApprox(depthScalar), deep, glow, core);
      color.nscale8_video(qadd8(12, scale8(depthScalar, 198)));
      limitLavaColor(color);
      SEGMENT.setPixelColorXY(x, y, color);
    }
  }
}

static const SceneDefinition LAVA_SCENE = {
  buildLavaField,
  lavaPipelineSettings,
  outputLava
};

static void renderLava(RenderState& state, const Surface& surface, uint16_t dt) {
  renderScene(state, surface, dt, LAVA_SCENE);
}

} // namespace CylinderLamp
