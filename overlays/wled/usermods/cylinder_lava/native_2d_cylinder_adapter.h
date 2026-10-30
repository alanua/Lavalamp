#pragma once
#include <stdint.h>
#include <math.h>
#include <string.h>

// Source manifest: WLED v16.0.1, 29b389df1c1aaec6ff53aea742d17063b985906c.
// The 53 addEffect registrations in the WLED_DISABLE_2D block of FX.cpp,
// including both classic and particle entries (WLED_PS_DONT_REPLACE_2D_FX).
namespace CylinderNative {
enum Mapping : uint8_t { FLAT_WRAP, RADIAL_BOTTOM_POLAR };
struct Entry { uint8_t id; const char* name; Mapping mapping; };
static constexpr Entry effects[] = {
  {114, "Rotozoomer", FLAT_WRAP}, // FX_MODE_2DPLASMAROTOZOOM
  {118, "Spaceships", FLAT_WRAP}, // FX_MODE_2DSPACESHIPS
  {119, "Crazy Bees", FLAT_WRAP}, // FX_MODE_2DCRAZYBEES
  {120, "Ghost Rider", FLAT_WRAP}, // FX_MODE_2DGHOSTRIDER
  {121, "Blobs", FLAT_WRAP}, // FX_MODE_2DBLOBS
  {122, "Scrolling Text", FLAT_WRAP}, // FX_MODE_2DSCROLLTEXT
  {123, "Drift Rose", RADIAL_BOTTOM_POLAR}, // FX_MODE_2DDRIFTROSE
  {124, "Distortion Waves", FLAT_WRAP}, // FX_MODE_2DDISTORTIONWAVES
  {139, "GEQ", FLAT_WRAP}, // FX_MODE_2DGEQ
  {146, "Noise2D", FLAT_WRAP}, // FX_MODE_2DNOISE
  {149, "Firenoise", FLAT_WRAP}, // FX_MODE_2DFIRENOISE
  {150, "Squared Swirl", FLAT_WRAP}, // FX_MODE_2DSQUAREDSWIRL
  {152, "DNA", FLAT_WRAP}, // FX_MODE_2DDNA
  {153, "Matrix", FLAT_WRAP}, // FX_MODE_2DMATRIX
  {154, "Metaballs", FLAT_WRAP}, // FX_MODE_2DMETABALLS
  {160, "Funky Plank", FLAT_WRAP}, // FX_MODE_2DFUNKYPLANK
  {162, "Pulser", FLAT_WRAP}, // FX_MODE_2DPULSER
  {164, "Drift", RADIAL_BOTTOM_POLAR}, // FX_MODE_2DDRIFT
  {165, "Waverly", FLAT_WRAP}, // FX_MODE_2DWAVERLY
  {166, "Sun Radiation", RADIAL_BOTTOM_POLAR}, // FX_MODE_2DSUNRADIATION
  {167, "Colored Bursts", FLAT_WRAP}, // FX_MODE_2DCOLOREDBURSTS
  {168, "Julia", FLAT_WRAP}, // FX_MODE_2DJULIA
  {172, "Game Of Life", FLAT_WRAP}, // FX_MODE_2DGAMEOFLIFE
  {173, "Tartan", FLAT_WRAP}, // FX_MODE_2DTARTAN
  {174, "Polar Lights", FLAT_WRAP}, // FX_MODE_2DPOLARLIGHTS
  {175, "Swirl", FLAT_WRAP}, // FX_MODE_2DSWIRL
  {176, "Lissajous", FLAT_WRAP}, // FX_MODE_2DLISSAJOUS
  {177, "Frizzles", FLAT_WRAP}, // FX_MODE_2DFRIZZLES
  {178, "Plasma Ball", RADIAL_BOTTOM_POLAR}, // FX_MODE_2DPLASMABALL
  {180, "Hiphotic", FLAT_WRAP}, // FX_MODE_2DHIPHOTIC
  {181, "Sindots", FLAT_WRAP}, // FX_MODE_2DSINDOTS
  {182, "DNA Spiral", FLAT_WRAP}, // FX_MODE_2DDNASPIRAL
  {183, "Black Hole", RADIAL_BOTTOM_POLAR}, // FX_MODE_2DBLACKHOLE
  {125, "Soap", FLAT_WRAP}, // FX_MODE_2DSOAP
  {126, "Octopus", RADIAL_BOTTOM_POLAR}, // FX_MODE_2DOCTOPUS
  {127, "Waving Cell", FLAT_WRAP}, // FX_MODE_2DWAVINGCELL
  {186, "Akemi", FLAT_WRAP}, // FX_MODE_2DAKEMI
  {187, "PS Volcano", FLAT_WRAP}, // FX_MODE_PARTICLEVOLCANO
  {188, "PS Fire", FLAT_WRAP}, // FX_MODE_PARTICLEFIRE
  {189, "PS Fireworks", FLAT_WRAP}, // FX_MODE_PARTICLEFIREWORKS
  {190, "PS Vortex", RADIAL_BOTTOM_POLAR}, // FX_MODE_PARTICLEVORTEX
  {191, "PS Fuzzy Noise", FLAT_WRAP}, // FX_MODE_PARTICLEPERLIN
  {192, "PS Ballpit", FLAT_WRAP}, // FX_MODE_PARTICLEPIT
  {193, "PS Box", FLAT_WRAP}, // FX_MODE_PARTICLEBOX
  {194, "PS Attractor", RADIAL_BOTTOM_POLAR}, // FX_MODE_PARTICLEATTRACTOR
  {195, "PS Impact", FLAT_WRAP}, // FX_MODE_PARTICLEIMPACT
  {196, "PS Waterfall", FLAT_WRAP}, // FX_MODE_PARTICLEWATERFALL
  {197, "PS Spray", FLAT_WRAP}, // FX_MODE_PARTICLESPRAY
  {198, "PS GEQ 2D", FLAT_WRAP}, // FX_MODE_PARTICLESGEQ
  {199, "PS GEQ Nova", RADIAL_BOTTOM_POLAR}, // FX_MODE_PARTICLECENTERGEQ
  {200, "PS Ghost Rider", FLAT_WRAP}, // FX_MODE_PARTICLEGHOSTRIDER
  {201, "PS Blobs", FLAT_WRAP}, // FX_MODE_PARTICLEBLOBS
  {217, "PS Galaxy", RADIAL_BOTTOM_POLAR}, // FX_MODE_PARTICLEGALAXY
};
static constexpr unsigned width = 16, height = 16, count = width * height;
inline const Entry* find(unsigned id) {
  for (const auto& e : effects) if (e.id == id) return &e;
  return nullptr;
}
inline int wrap(int x) { return ((x % 16) + 16) % 16; }
struct Coord { float x, y; };
inline Coord sourceCoord(Mapping mapping, float x, float y) {
  x -= floorf(x / 16.0f) * 16.0f;
  if (mapping == FLAT_WRAP) return {x, y};
  const float h = fmaxf(0.0f, fminf(1.0f, (15.0f - y) / 15.0f));
  const float radius = 7.5f * h;
  const float theta = x * (6.283185307179586f / 16.0f);
  // At h=0 all angles sample exactly (7.5,7.5), avoiding atan2(0,0).
  return {7.5f + radius * cosf(theta), 7.5f + radius * sinf(theta)};
}
inline uint32_t bilinear(const uint32_t* pixels, float x, float y) {
  x -= floorf(x / 16.0f) * 16.0f;
  y = fmaxf(0.0f, fminf(15.0f, y));
  const int x0 = int(floorf(x)), y0 = int(floorf(y));
  const int x1 = wrap(x0 + 1), y1 = y0 < 15 ? y0 + 1 : 15;
  const float fx = x - x0, fy = y - y0;
  uint32_t result = 0;
  for (unsigned shift = 0; shift < 32; shift += 8) {
    const float a = float((pixels[y0*16+x0] >> shift) & 255);
    const float b = float((pixels[y0*16+x1] >> shift) & 255);
    const float c = float((pixels[y1*16+x0] >> shift) & 255);
    const float d = float((pixels[y1*16+x1] >> shift) & 255);
    result |= uint32_t((a + (b-a)*fx)*(1-fy) + (c+(d-c)*fx)*fy + 0.5f) << shift;
  }
  return result;
}
inline uint32_t sample(const uint32_t* pixels, Mapping mapping, float x, float y) {
  const Coord p = sourceCoord(mapping, x, y);
  return bilinear(pixels, p.x, p.y);
}
// CY registrations are discovered at setup, never inferred from numeric offsets.
inline uint8_t cyIds[17] = {};
inline const char* const cyNames[] = {
"CY Anemone", "CY Tidal Bloom", "CY Lava Lamp", "CY Flame",
"CY Plasma Core", "CY Deep Noise", "CY Aurora Tube", "CY Inner Swirl",
"CY Bubbles Volume", "CY Ring Ripples", "CY Ring Ripples Rainbow",
"CY Bottom Rays", "CY Rising Bands", "CY Helical Plasma",
"CY Noise Waves Tube", "CY Cell Membrane Flow", "CY Cross Bands Tube"
};
inline void registerCyIdentity(unsigned index, uint8_t id) { if (index < 17) cyIds[index] = id; }
inline int resolveCyIdentity(const char* name) {
  if (!name) return -1;
  for (unsigned i=0; i<17; ++i)
    if (strcmp(name, cyNames[i]) == 0 && cyIds[i]) return cyIds[i];
  return -1; // fail closed: never substitute a stale numeric ID
}
inline bool isCy(unsigned id) {
  for (auto value : cyIds) if (value && value == id) return true;
  return false;
}
inline bool active(unsigned id, unsigned w, unsigned h) {
  return w == 16 && h == 16 && (find(id) || isCy(id));
}
inline uint32_t output(const uint32_t* pixels, unsigned id, unsigned w, unsigned h, int x, int y) {
  const Entry* e = find(id);
  if (e && w == 16 && h == 16) return sample(pixels, e->mapping, float(x), float(y));
  return pixels[y*w+x];
}
} // namespace CylinderNative

