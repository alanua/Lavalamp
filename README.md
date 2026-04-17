# Lavalamp

Stage 1 firmware work for a premium atmospheric cylinder lamp based on WLED.

This repository does not replace WLED. It fetches official WLED stable and overlays a small WLED usermod that registers one custom product effect: `Lava Lamp`.

## Stage 1 Scope

Implemented:

- Cylinder mapping layer
- Fixed render buffers
- One WLED-native custom effect: `Lava Lamp`

Not implemented in Stage 1:

- Flame
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

- `usermods/cylinder_lava/cylinder_lava_engine.h`
- `usermods/cylinder_lava/usermod_cylinder_lava.h`
- `platformio_override.ini`
- `wled00/usermods_list.cpp` registration hook only

The WLED core systems for OTA, UI, network, and presets are not modified.

## Render Model

The lava renderer uses a scalar field, not particles.

Pipeline:

1. Raw scalar field
2. Temporal smoothing: `smoothed = old * alpha + raw * (1 - alpha)`
3. Spatial blur with mandatory X wrap
4. Warm lava palette mapping
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
