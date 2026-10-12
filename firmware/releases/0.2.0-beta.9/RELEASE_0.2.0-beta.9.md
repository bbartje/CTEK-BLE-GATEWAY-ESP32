# CTEK ESP32 Gateway 0.2.0-beta.9

Prerelease for a limited hardware test. Owner and maintainer: **bbartje**.
The ESP32-WROOM-32 gateway reads up to four CTEK Battery Sense sensors and
sends their measurements to the tester's own MQTT broker and Home Assistant.
The owner's beta.9 hardware test is in progress; no complete acceptance
result or second-tester result has been reported yet.

Start with the [tester guide](TESTER_START.md) (Dutch), the
[installation guide](FLASHEN.md) (English), and the
[feedback template](TESTVERSLAG_TEMPLATE.md). The complete ZIP includes these
documents, the acceptance checklist, release notes, build manifest, checksums,
and library licenses. PlatformIO and PuTTY are not needed to install it.

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

- Empty sensor forms open one at a time through `+`. Configured sensors stay
  visible; details and settings start closed.
- The interface supports Dutch, English, and German and shows the last
  successful reading in the selected local time zone.
- A two-minute setup-network recovery window allows password recovery after
  startup. Normal network access and updates require the private admin password.

This publication completes the documentation and tester package. Its OTA and
factory binaries are byte for byte identical to the beta.9 files already
provided for the owner's current test.

## Install

Download from the fixed
[beta.9 release](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/tag/v0.2.0-beta.9).
From beta.1–beta.8, use the
[beta.9 OTA image](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.9/ctek-esp32-wroom-32-ota.bin)
and follow the [OTA guide](FLASHEN.md#5-update-firmware-over-wi-fi). Check the
version is **0.2.0-beta.9**. Wi-Fi, MQTT, sensors, and the admin password are
preserved. After the first successful reading, compare the Bluetooth dBm
value on the card and in Home Assistant. Discovery announces it again;
removing and re-adding the gateway is unnecessary.

For a first installation on **ESP32-WROOM-32 / ESP32-D0WD-V3 with 4 MB flash**,
use `ctek-esp32-wroom-32-factory.bin` at address **0x0**. A factory flash erases
existing settings. These images are for this ESP32 target, not C3/S2/S3.

## Verification and Remaining Tests

Host tests check RSSI error/default values and valid reception values, along
with the existing telemetry, protocol, access, and time-zone checks. The ESP32
build and release checks validate the actual binaries, checksums, flash layout,
ZIP contents, and correspondence to the source. Existing browser tests check
language selection and progressive sensor controls.

The restored Home Assistant entity, actual readings, radio range, recovery,
and OTA still require physical acceptance. The owner records the results
before inviting the second tester to use the same fixed release on their own
hardware. Use the [acceptance checklist](ACCEPTATIETEST.md) and return the
[feedback template](TESTVERSLAG_TEMPLATE.md). Only completed tests are marked
passed; skipped scenarios remain not tested.

## Known Limitations

Use a trusted home network. HTTP, MQTT, and stored settings are unencrypted;
a nearby person can reset the password during the public two-minute recovery
window. Keep codes and passwords private and anonymize diagnostic exports
before sharing them. Each tester manages their own Home Assistant backups.

RSSI updates with successful reads. Home Assistant stores history from live
messages; old gateway records are not automatically backfilled. The firmware
does not provide CTEK's calculated state-of-charge percentage. There is no
automatic boot rollback after an update, so USB recovery can be needed.

This is a prerelease for testing. Version 1.0.0 follows successful independent
installation, readings, recovery, and update tests on testers' own equipment.
