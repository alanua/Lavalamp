#pragma once

#include "cylinder_scene_contract.h"

namespace CylinderLamp {

struct LavaBlobSpec {
  uint8_t phaseOffset = 0;
  uint8_t angle = 0;
  uint8_t depthOffset = 0;
  uint8_t baseSize = 0;
  uint8_t heat = 0;

  LavaBlobSpec() {}
  LavaBlobSpec(uint8_t phase, uint8_t angularPosition, uint8_t depthPhase, uint8_t size, uint8_t blobHeat)
    : phaseOffset(phase),
      angle(angularPosition),
      depthOffset(depthPhase),
      baseSize(size),
      heat(blobHeat) {
  }
};

struct LavaBlob {
  uint16_t x = 0;       // Q8 angular position around the cylinder
  uint16_t y = 0;       // Q8 logical row position
  uint16_t radius = 0;  // Q8 projected radius
  uint8_t depth = 0;    // 0 = visually deeper in volume, 255 = closer/brighter
  uint8_t weight = 0;
};

static const uint8_t LAVA_PRIMARY_BLOB_COUNT = 2;

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

static inline uint16_t wrapCenterXQ8(int32_t x, const Surface& surface) {
  const int32_t circumference = int32_t(surface.width) << 8;
  if (circumference <= 0) return 0;
  while (x < 0) x += circumference;
  while (x >= circumference) x -= circumference;
  return uint16_t(x);
}

static inline uint8_t mapRange8(uint8_t value, uint8_t inMax) {
  if (inMax == 0) return 0;
  return uint8_t((uint16_t(value) * 255U) / inMax);
}

static inline void limitLavaColor(CRGB& color) {
  limitColor(color, 238, 158, 18, 345);
}

static MotionRates lavaMotionRates(const SceneControls& controls) {
  MotionRates rates;
  rates.angular = 6 + scale8(controls.flow, 18);
  rates.rise = 12 + scale8(controls.flow, 26);
  rates.rotation = 4 + scale8(controls.flow, 8);
  rates.deform = 3 + scale8(controls.softness, 6);
  return rates;
}

static PipelineSettings lavaPipelineSettings(const SceneContext& context) {
  PipelineSettings settings;
  settings.temporalAlpha = 188 + scale8(context.controls.viscosity, 44);
  settings.blurX = 18 + scale8(context.controls.softness, 26);
  settings.blurY = 18 + scale8(context.controls.softness, 22);
  settings.floor = 10;
  settings.ceiling = 230;
  settings.contrast = 170;
  return settings;
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

static inline uint8_t blobLifeEnvelope(uint8_t progress) {
  if (progress < 46) return scale8(ease8InOutApprox(mapRange8(progress, 46)), 230);
  if (progress > 218) return scale8(ease8InOutApprox(mapRange8(255 - progress, 37)), 230);
  return 230;
}

static inline uint16_t blobYQ8(const SceneContext& context, uint8_t progress) {
  const uint16_t bottom = uint16_t(context.surface.height - 1) << 8;
  const uint16_t top = 1 << 8;
  const uint8_t lift = ease8InOutApprox(scale8(progress, 232));
  return uint16_t(bottom - ((uint32_t(lift) * (bottom - top)) / 255U));
}

static inline uint16_t blobXQ8(const SceneContext& context, const LavaBlobSpec& spec, uint8_t progress) {
  const int32_t circumference = int32_t(context.surface.width) << 8;
  const int32_t base = (int32_t(spec.angle) * circumference) >> 8;
  const int16_t buoyantLean = int16_t(sin8_t(progress + spec.angle)) - 128;
  const int16_t depthSway = int16_t(sin8_t(phase8(context.motion.anglePhase) + spec.depthOffset)) - 128;
  return wrapCenterXQ8(base + buoyantLean + depthSway, context.surface);
}

static inline uint8_t blobDepth8(const SceneContext& context, const LavaBlobSpec& spec, uint8_t progress) {
  const uint8_t depth = sin8_t(phase8(context.motion.rotationPhase) + spec.depthOffset + (progress >> 2));
  return 96 + scale8(depth, 112);
}

static LavaBlob buildBlob(const SceneContext& context, const LavaBlobSpec& spec) {
  const uint8_t progress = phase8(context.motion.risePhase + (uint16_t(spec.phaseOffset) << 8));
  const uint8_t envelope = blobLifeEnvelope(progress);
  const uint8_t depth = blobDepth8(context, spec, progress);

  const uint16_t baseRadius = 590;
  const uint16_t sizeRadius = scale8(context.controls.size, 150);
  const uint16_t specRadius = uint16_t(spec.baseSize) * 3U;
  const uint16_t depthRadius = scale8(depth, 80);
  uint16_t radius = baseRadius + sizeRadius + specRadius + depthRadius;
  radius = uint16_t((uint32_t(radius) * (168U + scale8(envelope, 86))) >> 8);

  LavaBlob blob;
  blob.x = blobXQ8(context, spec, progress);
  blob.y = blobYQ8(context, progress);
  blob.radius = radius;
  blob.depth = depth;
  blob.weight = scale8(scale8(spec.heat, envelope), 134 + scale8(depth, 58));
  return blob;
}

static inline uint8_t metaballContribution(uint8_t x, uint8_t y, const Surface& surface, const LavaBlob& blob) {
  if (blob.weight < 8) return 0;

  const uint8_t field = radialFalloffQ8(x, y, surface, blob.x, blob.y, blob.radius, 100);
  return scale8(field, blob.weight);
}

static inline uint8_t birthZoneField(uint8_t y, const SceneContext& context) {
  const uint8_t h = heightFromBottom8(y, context.surface.height);
  if (h > 90) return 0;

  const uint8_t lower = qsub8(108, scale8(h, 210));
  return scale8(lower, 38 + scale8(context.controls.heat, 42));
}

static void buildLavaField(SceneContext& context) {
  advanceMotion(context.motion, context.dt, lavaMotionRates(context.controls));

  static const LavaBlobSpec primarySpec[LAVA_PRIMARY_BLOB_COUNT] = {
    LavaBlobSpec(12, 42, 74, 18, 218),
    LavaBlobSpec(142, 180, 186, 10, 204)
  };
  LavaBlob primary[LAVA_PRIMARY_BLOB_COUNT];
  for (uint8_t i = 0; i < LAVA_PRIMARY_BLOB_COUNT; i++) {
    primary[i] = buildBlob(context, primarySpec[i]);
  }

  for (uint8_t y = 0; y < context.surface.height; y++) {
    for (uint8_t x = 0; x < context.surface.width; x++) {
      uint8_t field = birthZoneField(y, context);

      for (uint8_t i = 0; i < LAVA_PRIMARY_BLOB_COUNT; i++) {
        addLayer(field, metaballContribution(x, y, context.surface, primary[i]));
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
