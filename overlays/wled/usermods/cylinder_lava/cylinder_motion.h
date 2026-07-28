#pragma once

#include "cylinder_geometry.h"

namespace CylinderLamp {

struct MotionState {
  uint16_t anglePhase = 0;
  uint16_t risePhase = 0;
  uint16_t rotationPhase = 0;
  uint16_t deformPhase = 0;
};

struct MotionRates {
  uint8_t angular = 16;
  uint8_t rise = 12;
  uint8_t rotation = 10;
  uint8_t deform = 9;
};

static inline void resetMotion(MotionState& motion) {
  motion.anglePhase = 0x1200;
  motion.risePhase = 0x5800;
  motion.rotationPhase = 0xA400;
  motion.deformPhase = 0x2C00;
}

static inline uint8_t phase8(uint16_t phase) {
  return uint8_t(phase >> 8);
}

static inline void advanceMotion(MotionState& motion, uint16_t dt, const MotionRates& rates) {
  motion.anglePhase += (uint32_t(dt) * rates.angular) >> 4;
  motion.risePhase += (uint32_t(dt) * rates.rise) >> 4;
  motion.rotationPhase += (uint32_t(dt) * rates.rotation) >> 4;
  motion.deformPhase += (uint32_t(dt) * rates.deform) >> 4;
}

static inline uint8_t upwardDrift8(uint8_t heightFromBottom, const MotionState& motion, uint8_t offset = 0) {
  return sin8_t(heightFromBottom + phase8(motion.risePhase) + offset);
}

static inline uint8_t angularSwirl8(uint8_t angle, uint8_t verticalRoll, const MotionState& motion, uint8_t amount) {
  return sin8_t(angle + phase8(motion.rotationPhase) + scale8(verticalRoll, amount));
}

static inline uint8_t localDeform8(uint8_t angle, uint8_t heightFromBottom, const MotionState& motion, uint8_t amount) {
  const uint8_t waveA = sin8_t(angle + phase8(motion.deformPhase));
  const uint8_t waveB = cos8_t(heightFromBottom + phase8(motion.anglePhase));
  return scale8(avg8(waveA, waveB), amount);
}

static inline uint16_t orbitXQ8(const Surface& surface, const MotionState& motion, uint8_t phaseOffset, uint8_t wobble) {
  const uint8_t phase = phase8(motion.anglePhase) + phaseOffset;
  const uint8_t wobblePhase = phase8(motion.deformPhase) + phaseOffset;
  return uint16_t(sin8_t(phase + scale8(sin8_t(wobblePhase), wobble))) * surface.width;
}

static inline uint16_t riseYQ8(const Surface& surface, const MotionState& motion, uint8_t phaseOffset, uint8_t wobble) {
  const uint8_t phase = phase8(motion.risePhase) + phaseOffset;
  const uint8_t wobblePhase = phase8(motion.rotationPhase) + phaseOffset;
  return uint16_t(sin8_t(phase + scale8(cos8_t(wobblePhase), wobble))) * (surface.height - 1);
}

} // namespace CylinderLamp
