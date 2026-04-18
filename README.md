# Lavalamp

Cylinder lamp firmware work for a premium atmospheric lamp based on WLED.

This repository does not replace WLED. It fetches official WLED `v0.15.3` and overlays a small WLED usermod that registers custom product effects for the cylinder lamp.

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
- `wled00/usermods_list.cpp` registration hook only

WLED core systems for OTA, UI, network, and presets are not rewritten, but a minimal integration patch is applied to WLED's usermod registration list.

## Versioning / Integration Risk

This integration is pinned to WLED `v0.15.3`. The patch targets the `wled00/usermods_list.cpp` layout in that release, so updating WLED may require refreshing the patch before building.

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
