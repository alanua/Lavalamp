# Lavalamp

Lavalamp is a firmware and visual-effects project for a custom cylindrical LED lamp based on WLED.

The physical target is a matte cylindrical glass body with an internal diffuser film taken from an LED TV backlight/screen stack. The diffuser is used to blur the LED pixels and create a soft lava, bloom, fog, and liquid-light effect rather than a sharp pixel display.

Current hardware target:

- LED layout: cylindrical 16x16 matrix
- Controller class: ESP32 first, ESP8266 support only where technically practical
- Firmware base: WLED fork/overlay workflow
- Visual goal: autonomous non-reactive ambient effects for a matte