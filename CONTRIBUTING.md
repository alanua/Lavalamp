# Contributing

Lavalamp accepts focused source, test, documentation, and firmware-overlay improvements.

## Pull requests

Keep changes narrow and state whether they affect source-only behavior, firmware build output, or physical-device expectations. Add deterministic tests where practical. Do not combine unrelated visual changes, deployment changes, and repository cleanup in one PR.

## WLED boundary

The project overlays pinned upstream WLED rather than maintaining a private fork of the whole WLED core. Changes should minimize patch surface and document any upstream-version assumptions.

## Hardware claims

Do not describe an effect, pin mapping, current limit, OTA image, or rollback path as physically verified unless the corresponding device test actually occurred. Source/test validation and physical verification are separate states.

## Public safety

Use public-safe fixtures and configuration only. Do not publish credentials, private topology, Home Edge state, or device secrets.

## License status

No open-source license has been selected for this repository yet. Contributions should not add or change a repository license without an explicit maintainer decision.
