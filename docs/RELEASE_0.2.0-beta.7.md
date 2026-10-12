# CTEK ESP32 Gateway 0.2.0-beta.7

Beta.7 fixes the unused sensor forms on the management page. In beta.6, the
first empty slot was always visible. With three configured sensors, slot 4
therefore appeared immediately and the `+` button was hidden. The owner
reported this behavior after an OTA update, and browser tests reproduced it.

## Changes

- Only configured sensors are visible when the page opens. Every empty slot
  starts hidden, including the first slot on a new installation.
- Each click on `+` reveals one empty slot, in slot order, up to four sensors.
  When no hidden empty slots remain, the button disappears.
- Configured sensors stay visible. Refreshing the page hides empty forms that
  have not been saved; the sensor settings stored on the ESP32 are preserved.
- Empty forms are also hidden in the HTML sent by the ESP32, before the
  browser script starts.

Beta.7 includes the features and corrections described in the
[beta.6 release notes](RELEASE_0.2.0-beta.6.md).

## Install

For an existing beta.1–beta.6 gateway, download the
[beta.7 OTA image](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/raw/refs/heads/main/firmware/releases/0.2.0-beta.7/ctek-esp32-wroom-32-ota.bin)
and upload it through the gateway's firmware update page. Keep the ESP32
powered until it restarts. Check that the page reports **0.2.0-beta.7**.
Wi-Fi, MQTT, sensor settings, and the admin password are preserved.

For a first installation on ESP32-WROOM-32 with 4 MB flash, use the beta.7
factory image at address `0x0`. Follow the [installation guide](FLASHEN.md).

## Verification and Remaining Tests

Browser tests run the production script with no configured sensors, one
sensor, gaps between sensor slots, three configured sensors, and four
configured sensors. They check that every empty form starts hidden, each
click reveals one form, configured sensors stay visible, and the button
disappears when no hidden slots remain. Language selection and sensor
controls are also checked with browser storage blocked.

The ESP32 build, host tests, and release checks validate the new binaries,
checksums, flash layout, ZIP contents, and correspondence to the source.
The new interface and the complete acceptance checklist still need testing
on the owner's real ESP32. No successful beta.7 hardware test has been reported.
Use the [acceptance checklist](ACCEPTATIETEST.md) and record results in the
[test status](VERVOLGTEST.md) before opening external testing or releasing 1.0.0.
