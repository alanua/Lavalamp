# Security Policy

Lavalamp combines firmware, network/OTA behavior, upstream WLED code, and physical LEDs. Security reports should therefore distinguish source-code defects from actions that would affect a real device.

## Reporting

Do not publish credentials, private network topology, device identifiers, signing material, or a working exploit in a public issue. Prefer GitHub private vulnerability reporting when available; otherwise contact the maintainer privately through the GitHub profile.

## High-sensitivity areas

- OTA/update integrity and rollback;
- unintended network exposure;
- upstream WLED or dependency supply-chain changes;
- device identity confusion or flashing the wrong target;
- firmware that can corrupt configuration or leave the device unbootable;
- unsafe brightness/current assumptions that can stress hardware;
- verification that reports success without observing the actual device state.

## Source vs deployment

A source build or passing test is not proof that a physical lamp was safely updated. Live flashing belongs behind a separately approved Skeleton/Home Edge operation with backup, exact artifact identity, rollback, and post-flash verification.

## Secrets and private data

Do not commit Wi-Fi credentials, tokens, private IP inventories, private keys, Home Edge secrets, or private device-registry state.
