#pragma once

#include "cylinder_motion.h"
#include "cylinder_pipeline.h"

namespace CylinderLamp {

struct RenderState {
  uint16_t width = 0;
  uint16_t height = 0;
  uint32_t lastMs = 0;
  uint8_t initialized = 0;
  uint8_t sceneId = 0;
  MotionState motion;
  FieldBuffers field;
};

enum SceneId : uint8_t {
  SCENE_ID_LAVA = 1,
  SCENE_ID_FLAME = 2,
  SCENE_ID_ANEMONE = 3,
  SCENE_ID_PLASMA_CORE = 4,
  SCENE_ID_DEEP_NOISE = 5
};

struct SceneControls {
  uint8_t flow = 0;
  uint8_t size = 0;
  uint8_t heat = 0;
  uint8_t viscosity = 0;
  uint8_t softness = 0;
};

struct SceneContext {
  RenderState& state;
  const Surface& surface;
  FieldBuffers& field;
  MotionState& motion;
  SceneControls controls;
  uint16_t dt;

  SceneContext(RenderState& renderState, const Surface& activeSurface, uint16_t elapsed)
    : state(renderState),
      surface(activeSurface),
      field(renderState.field),
      motion(renderState.motion),
      controls(),
      dt(elapsed) {
  }
};

typedef void (*SceneBuildFn)(SceneContext& context);
typedef PipelineSettings (*ScenePipelineFn)(const SceneContext& context);
typedef void (*SceneOutputFn)(SceneContext& context);

struct SceneDefinition {
  SceneBuildFn buildRaw;
  ScenePipelineFn pipeline;
  SceneOutputFn output;
};

static inline SceneControls readControls() {
  SceneControls controls;
  controls.flow = SEGMENT.speed;
  controls.size = SEGMENT.intensity;
  controls.heat = SEGMENT.custom1;
  controls.viscosity = SEGMENT.custom2;
  controls.softness = SEGMENT.custom3;
  return controls;
}

static bool prepare(RenderState& state, Surface& surface) {
  if (!strip.isMatrix || !SEGMENT.is2D()) return false;

  const uint16_t width = SEGMENT.virtualWidth();
  const uint16_t height = SEGMENT.virtualHeight();
  const uint32_t count = uint32_t(width) * height;
  if (width < 2 || height < 2 || count == 0 || count > CYLINDER_LAVA_MAX_PIXELS) return false;

  surface.width = uint8_t(width);
  surface.height = uint8_t(height);
  surface.count = uint16_t(count);

  if (!state.initialized || state.width != width || state.height != height) {
    clearFields(state.field);
    resetMotion(state.motion);
    state.width = width;
    state.height = height;
    state.lastMs = strip.now;
    state.initialized = 1;
    state.sceneId = 0;
  }

  return true;
}

static inline void selectScene(RenderState& state, uint8_t sceneId) {
  if (state.sceneId == sceneId) return;
  clearFields(state.field);
  resetMotion(state.motion);
  state.lastMs = strip.now;
  state.sceneId = sceneId;
}

static uint16_t elapsedMs(RenderState& state) {
  const uint32_t now = strip.now;
  uint32_t dt = now - state.lastMs;
  state.lastMs = now;
  if (dt > 96) dt = 96;
  return uint16_t(dt);
}

static void renderScene(RenderState& state, const Surface& surface, uint16_t dt, const SceneDefinition& scene) {
  SceneContext context(state, surface, dt);
  context.controls = readControls();

  if (scene.buildRaw) scene.buildRaw(context);
  const PipelineSettings settings = scene.pipeline ? scene.pipeline(context) : PipelineSettings();
  runPipeline(context.field, surface, settings);
  if (scene.output) scene.output(context);
}

} // namespace CylinderLamp
