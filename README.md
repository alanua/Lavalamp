# Lavalamp

Lavalamp is the public firmware and visual-effects companion for a physical 16x16 cylindrical ESP32 lamp built on WLED.

The repository does not replace WLED. It pins official WLED `v16.0.1`, applies a small cylinder-specific overlay/usermod, and keeps source-side validation separate from live-device deployment. Skeleton/Home Edge may control and verify a physical lamp, but this repository itself contains the reusable public source and test surface.

Repository maturity: active experimental hardware/firmware work. Public `main` must not be assumed identical to a deployed device image, and open draft firmware PRs are not part of `main` until merged. No open-source license has been selected for this repository yet.

## Current Scope

Implemented:

- Cylinder mapping layer
- Fixed render buffers
- WLED-native custom effects: `Lava Lamp`, `Flame`

Not implemented in this branch:

- Presets
- WLED UI/network/OTA/preset core changes
- Standalone firmware

## Hardware Target

- ESP32
- WS2812B 16x16 matrix
- 256 LEDs
- Matrix wrapped around a 50 mm cylinder
- Matte glass diffuser around 90 mm outer diameter
- Lamp height around 160 mm

## WLED Integration

The overlay adds a usermod to WLED and registers the effect with WLED's native `strip.addEffect(255, ...)` extension mechanism.

Files applied inside WLED:

- `usermods/cylinder_lava/cylinder_debug.h`
- `usermods/cylinder_lava/cylinder_geometry.h`
- `usermods/cylinder_lava/cylinder_motion.h`
- `usermods/cylinder_lava/cylinder_pipeline.h`
- `usermods/cylinder_lava/cylinder_lava_engine.h`
- `usermods/cylinder_lava/cylinder_scene_contract.h`
- `usermods/cylinder_lava/flame_scene.h`
- `usermods/cylinder_lava/lava_scene.h`
- `usermods/cylinder_lava/usermod_cylinder_lava.h`
- `platformio_override.ini`
- `wled00/usermod.cpp` registration hook only

WLED core systems for OTA, UI, network, and presets are not rewritten, but a minimal integration patch is applied to WLED's usermod registration list.

## Versioning / Integration Risk

This integration is pinned to WLED `v16.0.1`, exact commit `29b389df1c1aaec6ff53aea742d17063b985906c`. Preparation rejects a different checkout before copying overlays. The patch targets the `wled00/usermod.cpp` layout in that release, so updating WLED may require refreshing the patch before building.

## Render Model

The custom renderers use scalar fields, not particles.

The `Flame` effect uses a lightweight 16x16 heat-column model: lower rows inject stable heat, each frame transports heat upward with small wrapped X diffusion, and the shaped heat maps to ember/orange/gold output.

Pipeline:

1. Raw scalar field
2. Temporal smoothing: `smoothed = old * alpha + raw * (1 - alpha)`
3. Spatial blur with mandatory X wrap
4. Warm product palette mapping
5. WLED 2D pixel output

The cylinder coordinate model is:

- `X`: angle around the cylinder
- `Y`: height
- X wraps in all sampling and blur paths
- WLED's 2D matrix setup handles physical serpentine wiring

## Build Instructions

Prerequisites:

- Git
- PlatformIO
- Node.js 20+ and npm, required by WLED's web asset build step

Prepare the WLED workspace:

```powershell
.\scripts\prepare-wled.ps1
```

Build:

```powershell
cd .\firmware\WLED
pio run -e cylinder_lava_esp32
```

Upload firmware:

```powershell
pio run -e cylinder_lava_esp32 -t upload
```

If your LED data pin is not GPIO2, edit `overlays/wled/platformio_override.ini`, rerun `.\scripts\prepare-wled.ps1`, then build again.

## WLED Matrix Setup

In WLED, configure one 2D matrix panel:

- Width: `16`
- Height: `16`
- Total LEDs: `256`
- Serpentine: match physical wiring
- Bottom start / right start / vertical: match physical wiring

The custom effect assumes the WLED logical matrix is correct, then treats the logical X axis as a wrapped cylinder angle.

## Hardware Validation Checklist

- Confirm WLED matrix size is `16x16` with `256` LEDs.
- Confirm matrix orientation and start corner match the physical wrap.
- Confirm serpentine direction matches the panel wiring.
- Check X seam continuity with no jump on wrap.
- Confirm the configured GPIO matches the LED data pin.
- Set brightness/current limits conservatively.
- Judge Lava/Flame softness and brightness through the diffuser.

Optional: temporarily build with `-D CYLINDER_DEBUG_PATTERN` to show center stripes and seam markers.

## Phase 1 native 2D classification

The production manifest in `native_2d_cylinder_adapter.h` classifies all 53
registrations in the pinned upstream `FX.cpp` 2D block: 10
`RADIAL_BOTTOM_POLAR` and 43 `FLAT_WRAP`. Coverage includes conditional legacy
entries (`WLED_PS_DONT_REPLACE_2D_FX`) and ParticleSystem2D entries
(`WLED_DISABLE_PARTICLESYSTEM2D`), regardless of build configuration.

Run `node --test tests/wled_version_pin.test.mjs tests/native_2d_cylinder_adapter.test.mjs`
for source validation. The test embeds the verbatim pinned registration block and
ID definitions, with source blob IDs and snapshot checksums for offline repeatability.
An upstream upgrade must update the source pin and inventory together; any added,
removed, duplicate, or unclassified registration fails coverage.

This phase covers only the upstream pin and classification. Retained runtime
adapter and CY renderer work is deferred, and its host runtime test is explicitly
skipped in this focused suite. Firmware builds and device operations are separate.
