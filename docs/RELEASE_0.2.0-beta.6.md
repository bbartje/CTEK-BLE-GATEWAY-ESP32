# CTEK ESP32 Gateway 0.2.0-beta.6

Beta.6 updates the management interface and Home Assistant MQTT entities. It
also provides a time-limited local recovery path for installations whose admin
password has been forgotten.

## Changes

- A two-minute recovery access point starts alongside normal Wi-Fi, BLE, and
  MQTT startup after a configured gateway reboots. Existing settings are not
  erased. The public initial credential is accepted for admin recovery only
  during this window, only from the setup network and only for password
  recovery. Configuration, diagnostics, and OTA require the private admin
  credential. The access point closes at the fixed deadline. A nearby person
  can still reset the password during this window; the owner accepted that
  tradeoff for the beta.
- The web interface offers Dutch, English, and German and remembers the chosen
  language in the browser.
- Empty sensor slots are revealed progressively with a `+` control. The limit
  remains four sensors, read one at a time on the existing per-sensor interval.
- Home Assistant receives live voltage, current temperature, low-voltage state,
  and availability. Historical records remain available on the gateway MQTT
  history topic; Home Assistant can build its own history from live updates.
- Discovery removes the separate historical/current-temperature duplication
  and the RSSI entity. Existing voltage, temperature, and low-voltage entity
  identities are retained; the temperature entity now represents the current
  reading. The device manufacturer is `BartjeB`.
- The admin page shows the last successful read in the selected local time zone.
  Europe/Amsterdam is the default; other listed regions include their daylight-
  saving rules. MQTT timestamps remain Unix epoch values, which Home Assistant
  displays in its own configured time zone.
- The English flashing guide includes both first-time factory installation and
  OTA update instructions.
- Time synchronization preserves the chosen region and daylight-saving rules.
- Missing live voltage is reported as null; the low-voltage signal becomes
  unknown, and the web page labels the historical fallback explicitly.
- Language and sensor-slot controls continue working when browser storage is
  blocked. The `+` control also works before the first sensor is saved.
- Private admin passwords are no longer printed to the serial startup log.
- Builds apply the multipart boundary limit for CVE-2026-42854 to a build-local
  copy of Arduino WebServer, with no new runtime library or SDK-cache changes.

## Install

For a clean ESP32-WROOM-32 installation, use
`ctek-esp32-wroom-32-factory.bin` at address `0x0` with Espressif's Flash
Download Tool. For an existing beta.1–beta.5 gateway, upload
`ctek-esp32-wroom-32-ota.bin` from the gateway's update page. OTA preserves
configuration and the admin password. Follow [the flash and OTA guide](FLASHEN.md).

## Verification and Limitations

Host tests cover the session logic, recovery-window timeout arithmetic,
recovery access policy, real time-zone conversion, firmware-image validation,
telemetry policies, and the multipart boundary limit. Browser tests exercise
the production interface's language and sensor-slot controls. The
ESP32 build and release package must also pass CI. Physical BLE reads, live
voltage availability, MQTT delivery, and OTA on hardware still require testing
with a real ESP32 and Battery Sense sensor.

Beta.6 is the owner's final-test candidate. Complete the
[acceptance checklist](ACCEPTATIETEST.md) before opening external testing.
Keep the beta label during that phase; successful testing on external users'
own hardware and networks is required before version 1.0.0.
