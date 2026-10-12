# CTEK ESP32 Gateway 0.2.0-beta.8

Beta.8 gives configured sensors compact cards. The owner chose to show only
the sensor name, live voltage, and the last successful measurement time.

## Changes

- Each configured card shows the name, voltage, and measurement time in the
  selected local time zone. The layout wraps on smaller screens.
- **Details and settings** starts closed. It contains temperature, read status,
  counters, **Read now**, the BLE log, and the forms for editing or removing
  the sensor. Opening it uses the browser's built-in disclosure control.
- Missing live voltage is shown as **Unavailable**; old readings are labelled
  **stale**. Before the first successful reading, the time says **No reading yet**.
  Historical voltage remains explicitly labelled inside the details.
- Empty sensor slots still appear one at a time through `+`, as corrected in
  [beta.7](RELEASE_0.2.0-beta.7.md). New sensor forms open ready for entry.
- Refreshing the page closes the details again. Saved sensor configuration
  stays on the ESP32.

## Install

From beta.1–beta.7, download the
[beta.8 OTA image](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/raw/refs/heads/main/firmware/releases/0.2.0-beta.8/ctek-esp32-wroom-32-ota.bin)
and upload it through the gateway's firmware update page. You can update
directly from beta.6 or beta.7. Keep the ESP32 powered until it restarts and
check the version is **0.2.0-beta.8**. Wi-Fi, MQTT, sensors, and the admin
password are preserved. Follow the [installation and OTA guide](FLASHEN.md).

For a first installation on ESP32-WROOM-32 with 4 MB flash, use the beta.8
factory image at address `0x0`.

## Verification and Remaining Tests

The ESP32 build, host tests, and release checks validate the new binaries,
checksums, flash layout, ZIP, and correspondence to the source. Existing
browser tests check language selection, empty-slot controls, and blocked
browser storage. The compact layout uses native HTML details and CSS.

Check the cards and their disclosure controls on a real gateway, including
missing or stale voltage, local times, language changes, and a phone-sized
screen. Verify **Read now** and editing after expanding a card. No successful
beta.8 hardware acceptance test has been reported yet. Use the
[acceptance checklist](ACCEPTATIETEST.md) and record results in the
[test status](VERVOLGTEST.md) before external testing or a stable release.
