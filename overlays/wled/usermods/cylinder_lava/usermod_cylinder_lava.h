#pragma once

#include "wled.h"
#include "FX.h"
#include "cylinder_lava_engine.h"

typedef void (*CylinderRenderFn)(CylinderLamp::RenderState& state, const CylinderLamp::Surface& surface, uint16_t dt);

static constexpr uint8_t SCENE_ID_CY_TIDAL_BLOOM = 17;

struct TidalBloomState {
  uint32_t rng = 0;
  uint32_t nextTargetMs = 0;
  float thetaOffset = 0.0f;
  float thetaTarget = 0.0f;
  float heightOffset = 0.0f;
  float heightTarget = 0.0f;
  float scaleX = 1.0f;
  float scaleXTarget = 1.0f;
  float scaleY = 1.0f;
  float scaleYTarget = 1.0f;
  float phaseOffset = 0.0f;
  float phaseTarget = 0.0f;
  uint8_t initialized = 0;
};

struct CylinderRuntimeState {
  CylinderLamp::RenderState render;
  TidalBloomState tidalBloom;
};

static TidalBloomState* activeTidalBloomState = nullptr;

static uint16_t render_cylinder_scene(uint8_t sceneId, CylinderRenderFn renderFn);
static uint16_t mode_cy_anemone();
static uint16_t mode_cy_tidal_bloom();
static uint16_t mode_cy_lava_lamp();
static uint16_t mode_cy_flame();
static uint16_t mode_cy_plasma_core();
static uint16_t mode_cy_deep_noise();
static uint16_t mode_cy_aurora_tube();
static uint16_t mode_cy_inner_swirl();
static uint16_t mode_cy_bubbles_volume();
static uint16_t mode_cy_ring_ripples();
static uint16_t mode_cy_ring_ripples_rainbow();
static uint16_t mode_cy_bottom_rays();
static uint16_t mode_cy_rising_bands();
static uint16_t mode_cy_helical_plasma();
static uint16_t mode_cy_noise_waves_tube();
static uint16_t mode_cy_cell_membrane_flow();
static uint16_t mode_cy_cross_bands_tube();

static const char _data_FX_MODE_CY_ANEMONE[] PROGMEM =
  "CY Anemone@Flow,X Scale,Y Scale,Stability,Legs;Liquid,Organism,Tip;!;02;m12=0,sx=96,ix=51,c1=51,c2=160,c3=108";

static const char _data_FX_MODE_CY_TIDAL_BLOOM[] PROGMEM =
  "CY Tidal Bloom@Flow,X Scale,Y Scale,Variation,Legs;Liquid,Bloom,Tip;!;02;m12=0,sx=96,ix=51,c1=51,c2=160,c3=108";

static const char _data_FX_MODE_CY_LAVA_LAMP[] PROGMEM =
  "CY Lava Lamp@Flow,Scale,Energy,Stability,Softness;Liquid,Wax,Core;!;02;m12=0,sx=46,ix=150,c1=170,c2=190,c3=120";

static const char _data_FX_MODE_CY_FLAME[] PROGMEM =
  "CY Flame@Flow,Scale,Energy,Stability,Softness;Ember,Flame,Core;!;02;m12=0,sx=150,ix=130,c1=206,c2=112,c3=60";

static const char _data_FX_MODE_CY_PLASMA_CORE[] PROGMEM =
  "CY Plasma Core@Flow,Scale,Energy,Stability,Softness;Void,Filament,Core;!;02;m12=0,sx=124,ix=132,c1=196,c2=116,c3=76";

static const char _data_FX_MODE_CY_DEEP_NOISE[] PROGMEM =
  "CY Deep Noise@Flow,Scale,Energy,Stability,Softness;Depth,Body,Highlight;!;02;m12=0,sx=76,ix=150,c1=172,c2=180,c3=132";

static const char _data_FX_MODE_CY_AURORA_TUBE[] PROGMEM =
  "CY Aurora Tube@Flow,Scale,Energy,Stability,Softness;Low,Glow,Top;!;02;m12=0,sx=88,ix=128,c1=176,c2=150,c3=120";

static const char _data_FX_MODE_CY_INNER_SWIRL[] PROGMEM =
  "CY Inner Swirl@Flow,Scale,Energy,Stability,Softness;Depth,Swirl,Core;!;02;m12=0,sx=106,ix=128,c1=180,c2=140,c3=110";

static const char _data_FX_MODE_CY_BUBBLES_VOLUME[] PROGMEM =
  "CY Bubbles Volume@Flow,Scale,Energy,Stability,Softness;Depth,Bubble,Highlight;!;02;m12=0,sx=72,ix=128,c1=178,c2=160,c3=128";

static const char _data_FX_MODE_CY_RING_RIPPLES[] PROGMEM =
  "CY Ring Ripples@Flow,Scale,Energy,Stability,Softness;Depth,Ripple,Highlight;!;02;m12=0,sx=100,ix=128,c1=180,c2=130,c3=96";

static const char _data_FX_MODE_CY_RING_RIPPLES_RAINBOW[] PROGMEM =
  "CY Ring Ripples Rainbow@Flow,Scale,Energy,Stability,Softness;!;!;02;m12=0,sx=100,ix=128,c1=180,c2=130,c3=96";

static const char _data_FX_MODE_CY_BOTTOM_RAYS[] PROGMEM =
  "CY Bottom Rays@Flow,Scale,Energy,Stability,Softness;Root,Ray,Tip;!;02;m12=0,sx=104,ix=128,c1=188,c2=132,c3=84";

static const char _data_FX_MODE_CY_RISING_BANDS[] PROGMEM =
  "CY Rising Bands@Flow,Scale,Energy,Stability,Softness;!;!;02;m12=0,sx=120,ix=128,c1=180,c2=130,c3=100";

static const char _data_FX_MODE_CY_HELICAL_PLASMA[] PROGMEM =
  "CY Helical Plasma@Flow,Scale,Energy,Stability,Softness;Void,Helix,Core;!;02;m12=0,sx=112,ix=128,c1=190,c2=124,c3=90";

static const char _data_FX_MODE_CY_NOISE_WAVES_TUBE[] PROGMEM =
  "CY Noise Waves Tube@Flow,Scale,Energy,Stability,Softness;Depth,Wave,Highlight;!;02;m12=0,sx=84,ix=128,c1=172,c2=150,c3=120";

static const char _data_FX_MODE_CY_CELL_MEMBRANE_FLOW[] PROGMEM =
  "CY Cell Membrane Flow@Flow,Scale,Energy,Stability,Softness;Depth,Membrane,Highlight;!;02;m12=0,sx=76,ix=128,c1=176,c2=152,c3=124";

static const char _data_FX_MODE_CY_CROSS_BANDS_TUBE[] PROGMEM =
  "CY Cross Bands Tube@Flow,Scale,Energy,Stability,Softness;Depth,Band,Accent;!;02;m12=0,sx=104,ix=128,c1=180,c2=132,c3=96";

static inline uint32_t tidalBloomRandom(TidalBloomState& state) {
  uint32_t x = state.rng;
  if (x == 0) x = 0xA3C59AC3u;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  state.rng = x;
  return x;
}

static inline float tidalBloomUnitRandom(TidalBloomState& state) {
  return float(tidalBloomRandom(state) & 0x00FFFFFFu) / 16777215.0f;
}

static inline float tidalBloomRandomRange(TidalBloomState& state, float low, float high) {
  return low + ((high - low) * tidalBloomUnitRandom(state));
}

static inline float tidalBloomApproach(float current, float target, float amount) {
  if (amount < 0.0f) amount = 0.0f;
  if (amount > 1.0f) amount = 1.0f;
  return current + ((target - current) * amount);
}

static void resetTidalBloomState(TidalBloomState& state) {
  state.rng = uint32_t(micros()) ^ (strip.now * 0x9E3779B9u) ^ 0xA3C59AC3u;
  if (state.rng == 0) state.rng = 0xA3C59AC3u;
  state.nextTargetMs = strip.now + 1200U;
  state.thetaOffset = 0.0f;
  state.thetaTarget = 0.0f;
  state.heightOffset = 0.0f;
  state.heightTarget = 0.0f;
  state.scaleX = 1.0f;
  state.scaleXTarget = 1.0f;
  state.scaleY = 1.0f;
  state.scaleYTarget = 1.0f;
  state.phaseOffset = 0.0f;
  state.phaseTarget = 0.0f;
  state.initialized = 1;
}

static void updateTidalBloomState(TidalBloomState& state, uint16_t dt) {
  if (!state.initialized) resetTidalBloomState(state);

  const uint32_t now = strip.now;
  if (int32_t(now - state.nextTargetMs) >= 0) {
    const float variation = 0.30f + (float(SEGMENT.custom2) / 255.0f) * 0.70f;
    state.thetaTarget = tidalBloomRandomRange(state, -0.42f, 0.42f) * variation;
    state.heightTarget = tidalBloomRandomRange(state, -0.055f, 0.075f) * variation;
    state.scaleXTarget = 1.0f + tidalBloomRandomRange(state, -0.12f, 0.12f) * variation;
    state.scaleYTarget = 1.0f + tidalBloomRandomRange(state, -0.10f, 0.10f) * variation;
    state.phaseTarget = tidalBloomRandomRange(state, -34.0f, 34.0f) * variation;
    state.nextTargetMs = now + 3500U + (tidalBloomRandom(state) % 7501U);
  }

  const float responseMs = 1800.0f + (float(SEGMENT.custom2) / 255.0f) * 2400.0f;
  const float amount = responseMs > 0.0f ? float(dt) / responseMs : 1.0f;
  state.thetaOffset = tidalBloomApproach(state.thetaOffset, state.thetaTarget, amount);
  state.heightOffset = tidalBloomApproach(state.heightOffset, state.heightTarget, amount);
  state.scaleX = tidalBloomApproach(state.scaleX, state.scaleXTarget, amount);
  state.scaleY = tidalBloomApproach(state.scaleY, state.scaleYTarget, amount);
  state.phaseOffset = tidalBloomApproach(state.phaseOffset, state.phaseTarget, amount);
}

static void renderCyTidalBloom(CylinderLamp::RenderState&, const CylinderLamp::Surface& surface, uint16_t dt) {
  if (activeTidalBloomState == nullptr) {
    SEGMENT.fill(SEGCOLOR(0));
    return;
  }
  TidalBloomState& bloom = *activeTidalBloomState;
  updateTidalBloomState(bloom, dt);

  const int W = int(surface.width);
  const int H = int(surface.height);
  const uint16_t step = uint16_t(int32_t(CylinderLamp::octopusStep(float(strip.now))) + int32_t(bloom.phaseOffset));
  const float scaleX = CylinderLamp::octopusScaleFromUi(SEGMENT.intensity) * bloom.scaleX;
  const float scaleY = CylinderLamp::octopusScaleFromUi(SEGMENT.custom1) * bloom.scaleY;
  const float heightOrigin = 0.14f + bloom.heightOffset;

  for (uint8_t x = 0; x < surface.width; x++) {
    for (uint8_t y = 0; y < surface.height; y++) {
      const CylinderLamp::FxCoord coord =
        CylinderLamp::fxCoordCylinderShell(x, y, W, H, bloom.thetaOffset, heightOrigin, scaleX, scaleY);
      const CylinderLamp::OctopusSample sample = CylinderLamp::octopusSampleFromCoord(coord, W, H);
      SEGMENT.setPixelColorXY(x, y, CylinderLamp::octopusKernel(sample, step, SEGMENT.custom3));
    }
  }
}

static uint16_t render_cylinder_scene(uint8_t sceneId, CylinderRenderFn renderFn) {
  if (!SEGENV.allocateData(sizeof(CylinderRuntimeState))) {
    SEGMENT.fill(SEGCOLOR(0));
    return FRAMETIME;
  }

  CylinderRuntimeState* runtime = reinterpret_cast<CylinderRuntimeState*>(SEGENV.data);
  CylinderLamp::RenderState* state = &runtime->render;
  CylinderLamp::Surface surface;
  if (!CylinderLamp::prepare(*state, surface)) {
    SEGMENT.fill(SEGCOLOR(0));
    return 350;
  }

#ifdef CYLINDER_DEBUG_PATTERN
  CylinderLamp::renderDebugPattern(surface);
  return FRAMETIME;
#endif

  const bool sceneChanged = state->sceneId != sceneId;
  CylinderLamp::selectScene(*state, sceneId);
  if (sceneChanged && sceneId == SCENE_ID_CY_TIDAL_BLOOM) {
    resetTidalBloomState(runtime->tidalBloom);
  }

  const uint16_t dt = CylinderLamp::elapsedMs(*state);
  activeTidalBloomState = sceneId == SCENE_ID_CY_TIDAL_BLOOM ? &runtime->tidalBloom : nullptr;
  renderFn(*state, surface, dt);
  activeTidalBloomState = nullptr;
  return FRAMETIME;
}

static uint16_t mode_cy_anemone() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_ANEMONE, CylinderLamp::renderCyAnemone);
}

static uint16_t mode_cy_tidal_bloom() {
  return render_cylinder_scene(SCENE_ID_CY_TIDAL_BLOOM, renderCyTidalBloom);
}

static uint16_t mode_cy_lava_lamp() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_LAVA, CylinderLamp::renderCyLavaLamp);
}

static uint16_t mode_cy_flame() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_FLAME, CylinderLamp::renderCyFlame);
}

static uint16_t mode_cy_plasma_core() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_PLASMA_CORE, CylinderLamp::renderCyPlasmaCore);
}

static uint16_t mode_cy_deep_noise() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_DEEP_NOISE, CylinderLamp::renderCyDeepNoise);
}

static uint16_t mode_cy_aurora_tube() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_AURORA_TUBE, CylinderLamp::renderCyAuroraTube);
}

static uint16_t mode_cy_inner_swirl() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_INNER_SWIRL, CylinderLamp::renderCyInnerSwirl);
}

static uint16_t mode_cy_bubbles_volume() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_BUBBLES_VOLUME, CylinderLamp::renderCyBubblesVolume);
}

static uint16_t mode_cy_ring_ripples() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_RING_RIPPLES, CylinderLamp::renderCyRingRipples);
}

static uint16_t mode_cy_ring_ripples_rainbow() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_RING_RIPPLES_RAINBOW, CylinderLamp::renderCyRingRipplesRainbow);
}

static uint16_t mode_cy_bottom_rays() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_BOTTOM_RAYS, CylinderLamp::renderCyBottomRays);
}

static uint16_t mode_cy_rising_bands() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_RISING_BANDS, CylinderLamp::renderCyRisingBands);
}

static uint16_t mode_cy_helical_plasma() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_HELICAL_PLASMA, CylinderLamp::renderCyHelicalPlasma);
}

static uint16_t mode_cy_noise_waves_tube() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_NOISE_WAVES_TUBE, CylinderLamp::renderCyNoiseWavesTube);
}

static uint16_t mode_cy_cell_membrane_flow() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_CELL_MEMBRANE_FLOW, CylinderLamp::renderCyCellMembraneFlow);
}

static uint16_t mode_cy_cross_bands_tube() {
  return render_cylinder_scene(CylinderLamp::SCENE_ID_CROSS_BANDS_TUBE, CylinderLamp::renderCyCrossBandsTube);
}

class CylinderLavaUsermod : public Usermod {
private:
  bool initDone = false;

public:
  void setup() override {
    if (initDone) return;
    strip.addEffect(255, &mode_cy_anemone, _data_FX_MODE_CY_ANEMONE);
    strip.addEffect(255, &mode_cy_tidal_bloom, _data_FX_MODE_CY_TIDAL_BLOOM);
    strip.addEffect(255, &mode_cy_lava_lamp, _data_FX_MODE_CY_LAVA_LAMP);
    strip.addEffect(255, &mode_cy_flame, _data_FX_MODE_CY_FLAME);
    strip.addEffect(255, &mode_cy_plasma_core, _data_FX_MODE_CY_PLASMA_CORE);
    strip.addEffect(255, &mode_cy_deep_noise, _data_FX_MODE_CY_DEEP_NOISE);
    strip.addEffect(255, &mode_cy_aurora_tube, _data_FX_MODE_CY_AURORA_TUBE);
    strip.addEffect(255, &mode_cy_inner_swirl, _data_FX_MODE_CY_INNER_SWIRL);
    strip.addEffect(255, &mode_cy_bubbles_volume, _data_FX_MODE_CY_BUBBLES_VOLUME);
    strip.addEffect(255, &mode_cy_ring_ripples, _data_FX_MODE_CY_RING_RIPPLES);
    strip.addEffect(255, &mode_cy_ring_ripples_rainbow, _data_FX_MODE_CY_RING_RIPPLES_RAINBOW);
    strip.addEffect(255, &mode_cy_bottom_rays, _data_FX_MODE_CY_BOTTOM_RAYS);
    strip.addEffect(255, &mode_cy_rising_bands, _data_FX_MODE_CY_RISING_BANDS);
    strip.addEffect(255, &mode_cy_helical_plasma, _data_FX_MODE_CY_HELICAL_PLASMA);
    strip.addEffect(255, &mode_cy_noise_waves_tube, _data_FX_MODE_CY_NOISE_WAVES_TUBE);
    strip.addEffect(255, &mode_cy_cell_membrane_flow, _data_FX_MODE_CY_CELL_MEMBRANE_FLOW);
    strip.addEffect(255, &mode_cy_cross_bands_tube, _data_FX_MODE_CY_CROSS_BANDS_TUBE);
    initDone = true;
  }

  void loop() override {
  }
};
