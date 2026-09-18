# Quickstart: validate the public source safely

This path validates the repository without flashing a device.

## Requirements

- Git
- Node.js 20+

## Run the deterministic tests

```bash
git clone https://github.com/alanua/Lavalamp.git
cd Lavalamp
node --test tests/*.test.mjs
```

The tests cover repository-owned generative visual contracts and Home Edge integration files. They do not contact a lamp, perform OTA, or prove physical LED output.

## Optional firmware preparation

Firmware preparation is documented in the main README and is pinned to WLED `v0.15.3`. Building firmware is a separate step from deploying it.

Do not make flashing or OTA the default quickstart. A real device update should first preserve configuration, bind an exact artifact/hash to the intended device, and have a rollback and observable postcondition.
