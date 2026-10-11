import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import test from 'node:test';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const includeDir = path.join(root, 'overlays', 'wled', 'usermods', 'cylinder_lava');

const wledStub = String.raw`
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

struct CRGB {
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
  uint8_t height = 12;
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
    if (index < 256U) pixels[index] = color;
  }
};

struct StripStub {
  bool isMatrix = true;
  uint32_t now = 0;
};

extern SegmentStub SEGMENT;
extern StripStub strip;

#define SEGCOLOR(slot) (SEGMENT.colors[(slot)])
`;

const cppHarness = String.raw`
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "lava_scene.h"

SegmentStub SEGMENT;
StripStub strip;

using CylinderLamp::RenderState;
using CylinderLamp::Surface;

static int fail(const std::string& message) {
  std::cerr << message << "\n";
  return 1;
}

static uint32_t energy(const CRGB& c) {
  return uint32_t(c.r) + c.g + c.b;
}

static uint32_t colorDelta(const CRGB& a, const CRGB& b) {
  return uint32_t(std::abs(int(a.r) - int(b.r))) +
         uint32_t(std::abs(int(a.g) - int(b.g))) +
         uint32_t(std::abs(int(a.b) - int(b.b)));
}

static uint32_t rowEnergy(uint8_t y, const Surface& surface) {
  uint32_t total = 0;
  for (uint8_t x = 0; x < surface.width; x++) {
    total += energy(SEGMENT.pixels[CylinderLamp::indexOf(x, y, surface)]);
  }
  return total;
}

static uint64_t checksum(const Surface& surface) {
  uint64_t hash = 1469598103934665603ULL;
  for (uint16_t i = 0; i < surface.count; i++) {
    const CRGB& c = SEGMENT.pixels[i];
    hash ^= c.r;
    hash *= 1099511628211ULL;
    hash ^= c.g;
    hash *= 1099511628211ULL;
    hash ^= c.b;
    hash *= 1099511628211ULL;
  }
  return hash;
}

static std::vector<CRGB> snapshot(const Surface& surface) {
  return std::vector<CRGB>(SEGMENT.pixels, SEGMENT.pixels + surface.count);
}

static void configureSurface(Surface& surface) {
  surface.width = 16;
  surface.height = 12;
  surface.count = uint16_t(surface.width) * surface.height;
  SEGMENT.width = surface.width;
  SEGMENT.height = surface.height;
  SEGMENT.speed = 92;
  SEGMENT.intensity = 140;
  SEGMENT.custom1 = 118;
  SEGMENT.custom2 = 164;
  SEGMENT.custom3 = 112;
  SEGMENT.colors[0] = 0x030001;
  SEGMENT.colors[1] = 0xE04800;
  SEGMENT.colors[2] = 0xFF8A08;
}

static void configureScene(Surface& surface,
                           uint8_t width,
                           uint8_t height,
                           uint8_t speed,
                           uint8_t intensity,
                           uint8_t heat,
                           uint8_t viscosity,
                           uint8_t softness,
                           uint32_t color0,
                           uint32_t color1,
                           uint32_t color2) {
  surface.width = width;
  surface.height = height;
  surface.count = uint16_t(surface.width) * surface.height;
  SEGMENT.width = surface.width;
  SEGMENT.height = surface.height;
  SEGMENT.speed = speed;
  SEGMENT.intensity = intensity;
  SEGMENT.custom1 = heat;
  SEGMENT.custom2 = viscosity;
  SEGMENT.custom3 = softness;
  SEGMENT.colors[0] = color0;
  SEGMENT.colors[1] = color1;
  SEGMENT.colors[2] = color2;
}

static uint64_t renderSequence(RenderState& state, const Surface& surface, uint8_t frames, std::vector<uint32_t>* frameDeltas) {
  std::vector<CRGB> previous;
  uint64_t combined = 0;

  for (uint8_t frame = 0; frame < frames; frame++) {
    SEGMENT.clearPixels();
    CylinderLamp::renderLava(state, surface, 33);
    const std::vector<CRGB> current = snapshot(surface);

    if (!previous.empty() && frameDeltas) {
      uint32_t maxDelta = 0;
      uint32_t totalDelta = 0;
      for (uint16_t i = 0; i < surface.count; i++) {
        const uint32_t delta = colorDelta(previous[i], current[i]);
        maxDelta = std::max(maxDelta, delta);
        totalDelta += delta;
      }
      frameDeltas->push_back((maxDelta << 16) | (totalDelta / surface.count));
    }

    combined ^= checksum(surface) + 0x9e3779b97f4a7c15ULL + (combined << 6) + (combined >> 2);
    previous = current;
  }

  return combined;
}

int main() {
  Surface surface;
  configureSurface(surface);

  if (CylinderLamp::heightFromBottom8(surface.height - 1, surface.height) != 0) {
    return fail("physical bottom row must map to zero height-from-bottom");
  }
  if (CylinderLamp::heightFromBottom8(0, surface.height) != 255) {
    return fail("top row must map to maximum height-from-bottom");
  }
  if (CylinderLamp::heightFromBottom8(surface.height - 2, surface.height) <=
      CylinderLamp::heightFromBottom8(surface.height - 1, surface.height)) {
    return fail("height-from-bottom must increase above the physical bottom row");
  }

  RenderState first;
  CylinderLamp::clearFields(first.field);
  CylinderLamp::resetMotion(first.motion);
  SEGMENT.clearPixels();
  CylinderLamp::renderLava(first, surface, 33);

  uint32_t seamTotal = 0;
  uint32_t adjacentTotal = 0;
  uint32_t adjacentCount = 0;
  for (uint8_t y = 0; y < surface.height; y++) {
    seamTotal += colorDelta(
      SEGMENT.pixels[CylinderLamp::indexOf(0, y, surface)],
      SEGMENT.pixels[CylinderLamp::indexOf(surface.width - 1, y, surface)]
    );
    for (uint8_t x = 0; x + 1 < surface.width; x++) {
      adjacentTotal += colorDelta(
        SEGMENT.pixels[CylinderLamp::indexOf(x, y, surface)],
        SEGMENT.pixels[CylinderLamp::indexOf(x + 1, y, surface)]
      );
      adjacentCount++;
    }
  }
  const uint32_t seamMean = seamTotal / surface.height;
  const uint32_t adjacentMean = adjacentTotal / adjacentCount;
  if (seamMean > adjacentMean * 2U + 8U) {
    return fail("cylinder seam is less continuous than ordinary horizontal neighbors");
  }

  RenderState a;
  CylinderLamp::clearFields(a.field);
  CylinderLamp::resetMotion(a.motion);
  std::vector<uint32_t> deltas;
  const uint64_t checksumA = renderSequence(a, surface, 7, &deltas);

  RenderState b;
  CylinderLamp::clearFields(b.field);
  CylinderLamp::resetMotion(b.motion);
  const uint64_t checksumB = renderSequence(b, surface, 7, nullptr);
  if (checksumA != checksumB) {
    return fail("native lava sequence must be deterministic from the same state and controls");
  }

  bool moved = false;
  for (const uint32_t packed : deltas) {
    const uint32_t maxDelta = packed >> 16;
    const uint32_t meanDelta = packed & 0xFFFFU;
    if (meanDelta > 0) moved = true;
    if (maxDelta > 70U || meanDelta > 18U) {
      return fail("native lava frame transition is too abrupt for the gentle scene contract");
    }
  }
  if (!moved) {
    return fail("native lava frames should transition, not freeze");
  }

  std::cout << "bottom_height=" << int(CylinderLamp::heightFromBottom8(surface.height - 1, surface.height))
            << " top_height=" << int(CylinderLamp::heightFromBottom8(0, surface.height))
            << " seam_mean=" << seamMean
            << " adjacent_mean=" << adjacentMean
            << " checksum=" << checksumA
            << "\n";

  struct Config {
    const char* name;
    uint8_t width;
    uint8_t height;
    uint8_t speed;
    uint8_t intensity;
    uint8_t heat;
    uint8_t viscosity;
    uint8_t softness;
    uint32_t color0;
    uint32_t color1;
    uint32_t color2;
  };

  const Config configs[] = {
    {"default_16x12", 16, 12, 92, 140, 118, 164, 112, 0x030001, 0xE04800, 0xFF8A08},
    {"compact_10x9_soft", 10, 9, 70, 96, 88, 210, 190, 0x030001, 0xD95700, 0xFFB018},
    {"narrow_wrap_7x11", 7, 11, 118, 170, 142, 150, 124, 0x030001, 0xE04800, 0xFF8A08},
    {"warm_user_palette", 15, 10, 145, 118, 174, 188, 84, 0x120201, 0xF06A08, 0xFFE040},
  };

  for (const Config& config : configs) {
    configureScene(surface,
                   config.width,
                   config.height,
                   config.speed,
                   config.intensity,
                   config.heat,
                   config.viscosity,
                   config.softness,
                   config.color0,
                   config.color1,
                   config.color2);

    if (CylinderLamp::heightFromBottom8(surface.height - 1, surface.height) != 0) {
      return fail(std::string(config.name) + ": physical bottom row must map to zero height-from-bottom");
    }
    if (CylinderLamp::heightFromBottom8(0, surface.height) != 255) {
      return fail(std::string(config.name) + ": top row must map to maximum height-from-bottom");
    }
    if (CylinderLamp::heightFromBottom8(surface.height - 2, surface.height) <=
        CylinderLamp::heightFromBottom8(surface.height - 1, surface.height)) {
      return fail(std::string(config.name) + ": height-from-bottom must increase above the physical bottom row");
    }

    RenderState firstConfigFrame;
    CylinderLamp::clearFields(firstConfigFrame.field);
    CylinderLamp::resetMotion(firstConfigFrame.motion);
    SEGMENT.clearPixels();
    CylinderLamp::renderLava(firstConfigFrame, surface, 33);

    uint32_t configSeamTotal = 0;
    uint32_t configAdjacentTotal = 0;
    uint32_t configAdjacentCount = 0;
    for (uint8_t y = 0; y < surface.height; y++) {
      configSeamTotal += colorDelta(
        SEGMENT.pixels[CylinderLamp::indexOf(0, y, surface)],
        SEGMENT.pixels[CylinderLamp::indexOf(surface.width - 1, y, surface)]
      );
      for (uint8_t x = 0; x + 1 < surface.width; x++) {
        configAdjacentTotal += colorDelta(
          SEGMENT.pixels[CylinderLamp::indexOf(x, y, surface)],
          SEGMENT.pixels[CylinderLamp::indexOf(x + 1, y, surface)]
        );
        configAdjacentCount++;
      }
    }

    const uint32_t configSeamMean = configSeamTotal / surface.height;
    const uint32_t configAdjacentMean = configAdjacentTotal / configAdjacentCount;
    if (configSeamMean > configAdjacentMean * 2U + 8U) {
      return fail(std::string(config.name) + ": cylinder seam is less continuous than ordinary horizontal neighbors");
    }

    RenderState configA;
    CylinderLamp::clearFields(configA.field);
    CylinderLamp::resetMotion(configA.motion);
    std::vector<uint32_t> configDeltas;
    const uint64_t configChecksumA = renderSequence(configA, surface, 9, &configDeltas);

    RenderState configB;
    CylinderLamp::clearFields(configB.field);
    CylinderLamp::resetMotion(configB.motion);
    const uint64_t configChecksumB = renderSequence(configB, surface, 9, nullptr);
    if (configChecksumA != configChecksumB) {
      return fail(std::string(config.name) + ": native lava sequence must be deterministic from the same state and controls");
    }

    bool configMoved = false;
    for (const uint32_t packed : configDeltas) {
      const uint32_t maxDelta = packed >> 16;
      const uint32_t meanDelta = packed & 0xFFFFU;
      if (meanDelta > 0) configMoved = true;
      if (maxDelta > 70U || meanDelta > 36U) {
        return fail(std::string(config.name) + ": native lava frame transition is too abrupt for the gentle scene contract"
          + " max_delta=" + std::to_string(maxDelta)
          + " mean_delta=" + std::to_string(meanDelta));
      }
    }
    if (!configMoved) {
      return fail(std::string(config.name) + ": native lava frames should transition, not freeze");
    }
  }

  return 0;
}
`;

const run = (command, args, options = {}) => {
  const result = spawnSync(command, args, { encoding: 'utf8', ...options });
  if (result.status !== 0) {
    throw new Error([
      `command failed: ${command} ${args.join(' ')}`,
      result.error ? String(result.error.message) : '',
      result.stdout?.trim() ?? '',
      result.stderr?.trim() ?? '',
    ].filter(Boolean).join('\n'));
  }
  return result.stdout ?? '';
};

test('native C++ lava scene preserves cylinder orientation, seam continuity, and gentle deterministic motion', () => {
  const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'lavalamp-native-'));
  try {
    fs.writeFileSync(path.join(tmp, 'wled.h'), wledStub);
    fs.writeFileSync(path.join(tmp, 'FX.h'), '#pragma once\n');
    fs.writeFileSync(path.join(tmp, 'cylinder_scene_native_harness.cpp'), cppHarness);

    const compiler = process.env.CXX || 'g++';
    const binary = path.join(tmp, 'cylinder_scene_native_harness');
    run(compiler, [
      '-std=c++17',
      '-I',
      tmp,
      '-I',
      includeDir,
      path.join(tmp, 'cylinder_scene_native_harness.cpp'),
      '-o',
      binary,
    ]);

    const output = run(binary, []);
    assert.match(output, /bottom_height=\d+ top_height=\d+ seam_mean=\d+ adjacent_mean=\d+ checksum=\d+/);
  } finally {
    fs.rmSync(tmp, { recursive: true, force: true });
  }
});
