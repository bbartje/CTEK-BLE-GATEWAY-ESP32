# CTEK ESP32 Gateway 0.2.0-beta.9

Beta.9 restores Bluetooth reception strength (RSSI) in Home Assistant and
makes it visible directly on the compact sensor cards. The owner requested
this to assess reception at first glance.

## Changes

- **Bluetooth-signaal** returns as a regular Home Assistant MQTT sensor with
  unit `dBm`, device class `signal_strength`, and zero display decimals.
  Its Discovery topic and unique ID match the original RSSI sensor.
- Startup Discovery republishes RSSI. Only the retired duplicate temperature
  entity is cleared; RSSI is removed when its sensor is actually deleted.
- Each configured card shows name, voltage, Bluetooth reception, and the
  last successful measurement time, with settings still collapsed.
- The RSSI comes from the Bluetooth connection used for the last successful
  read. For example, −60 dBm is stronger than −90 dBm. It is not a continuous
  radio monitor; the value updates with successful sensor reads.
- If RSSI is unavailable, Home Assistant receives `null` and the card shows
  **Unavailable**. NimBLE's error/default `0` and the controller's unknown
  marker `127` are not displayed as reception measurements. Old values on
  the card are labelled **stale**; Home Assistant uses the existing 30-minute
  expiry and availability policy.

Beta.9 includes the [beta.8 compact layout](RELEASE_0.2.0-beta.8.md) and
[beta.7 empty-slot correction](RELEASE_0.2.0-beta.7.md).

## Install

From beta.1–beta.8, download the
[beta.9 OTA image](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/raw/refs/heads/main/firmware/releases/0.2.0-beta.9/ctek-esp32-wroom-32-ota.bin)
and follow the [OTA guide](FLASHEN.md#5-update-firmware-over-wi-fi). Check the
version is **0.2.0-beta.9**. Wi-Fi, MQTT, sensors, and the admin password are
preserved. After the first successful reading, compare the Bluetooth dBm
value on the card and in Home Assistant. Discovery announces it again;
removing and re-adding the gateway is unnecessary.

For a first installation on ESP32-WROOM-32 with 4 MB flash, use the beta.9
factory image at address `0x0`.

## Verification and Remaining Tests

Host tests check RSSI error/default values and valid reception values, along
with the existing telemetry, protocol, access, and time-zone checks. The ESP32
build and release checks validate the actual binaries, checksums, flash layout,
ZIP contents, and correspondence to the source. Existing browser tests check
language selection and progressive sensor controls.

The restored Home Assistant entity and the card's real RSSI still require
testing with the owner's ESP32 and sensors. No beta.9 hardware result has been
reported. Use the [acceptance checklist](ACCEPTATIETEST.md) and preserve the
results in the [test status](VERVOLGTEST.md).
