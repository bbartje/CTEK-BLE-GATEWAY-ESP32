# CTEK Battery Sense → ESP32 → Home Assistant

Owner and maintainer: **bbartje**. Firmware version: **0.2.0-beta.10**.

[Open the beta.10 release](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/tag/v0.2.0-beta.10),
download the [Wi-Fi update (.bin)](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.10/ctek-esp32-wroom-32-ota.bin),
[complete factory image (.bin, flash address 0x0)](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.10/ctek-esp32-wroom-32-factory.bin),
or [firmware package (ZIP)](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.10/ctek-esp32-wroom-32-0.2.0-beta.10.zip).
Follow the [installation guide](docs/FLASHEN.md). The ZIP includes the tester
start guide, acceptance checklist, feedback template, release notes, checksums,
build information, and library licenses. Earlier releases and their original
packages remain available, including [beta.9](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/tag/v0.2.0-beta.9).

Beta.10 starts with **Gateway settings** collapsed. Open it to change Wi-Fi,
MQTT, time zone, polling interval, warning threshold, or the admin password.
Refresh, Bluetooth scan, diagnostics, and firmware update stay at the top while
scrolling. A cream/petrol/orange retro design and a visible **BartjeB / GitHub**
link complete the interface. Read the [beta.10 release notes](docs/RELEASE_0.2.0-beta.10.md).
Open [the offline interface preview](docs/VOORBEELD_INTERFACE.html) in a browser
to try the layout with fictional data, without a connected gateway.

Each configured card shows **name, live voltage, Bluetooth signal strength,
and last successful measurement time**. Temperature, read status, counters,
**Read now**, logs, and sensor settings are under **Details and settings**,
closed by default. Missing voltage or signal strength is shown as unavailable;
old values are labelled stale. Empty sensor forms appear only after clicking
**+**, one at a time. Dutch, English, and German are supported; the browser
remembers the language and measurement times use the configured local time zone.
Bluetooth signal strength (RSSI) is in dBm from the last successful read;
−60 dBm is stronger than −90 dBm.

The gateway publishes live voltage, current temperature, Bluetooth signal
strength, and low-voltage status to Home Assistant. The device is identified
as **BartjeB**. Historical records remain available on the MQTT history topic;
they do not automatically backfill the normal Home Assistant graphs.
The [ESPHome comparison](docs/VERGELIJKING_ESPHOME_CTEK.md) describes a possible
larger history import. Beta.10 retains the existing 12/48-record limits.

**Release status (October 12, 2026):** beta.10 is delivered as a GitHub
prerelease with fixed tag `v0.2.0-beta.10`. The source and complete installation
package pass the software checks. The OTA and factory images are byte for
byte identical to the beta.10 files already built for the owner's test;
this publication completes the release documentation and downloads.
Physical beta.10 installation, repeated reads, Home Assistant, recovery, and
OTA acceptance remain pending. The earlier confirmed physical result is a
complete read of **sensor slot 3 on beta.4**, including two history records
and reception in Home Assistant. No full beta.10 acceptance is inferred from it.

The [test status](docs/VERVOLGTEST.md) and
[beta.6 quality report](KWALITEITSRAPPORT-2026-10-12.md) record evidence and
limitations. Keep the beta label during testing; release 1.0.0 only after
installation, repeated reads, Home Assistant, recovery, and updates have passed
on testers' own devices.

## Update an Existing Gateway (OTA)

Download the [beta.10 OTA image](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.10/ctek-esp32-wroom-32-ota.bin).
On the gateway page, choose **Update firmware** (older versions:
**Pause polling and update firmware**), wait for the active sensor read to
finish, then upload `ctek-esp32-wroom-32-ota.bin`. Keep the ESP32 powered until
it restarts. Wi-Fi/MQTT settings, sensors, and the admin password are preserved.
Confirm version **0.2.0-beta.10**, closed **Gateway settings**, and the toolbar
while scrolling. Check a new reading on the gateway and in Home Assistant.
Do not upload the factory image here. See the
[complete OTA instructions](docs/FLASHEN.md#5-update-firmware-over-wi-fi).

**PlatformIO and PuTTY are not required to install this firmware.** For a
first factory flash, connect to `CTEK-Setup-xxxxxx` with the initial password
**CTEK-Start123**. Open **http://192.168.4.1**, sign in as **admin** with the
same initial password, and choose a personal password. After restarting,
reconnect with your new password, then open **Gateway settings** to configure
Wi-Fi/MQTT and add sensors through **+**. A factory flash erases old settings.
A two-minute password recovery network is available after startup; the public
recovery login only allows replacing the admin password on that setup network.
Normal configuration, diagnostics, and uploads require your own password.

## Start a Second Hardware Test

After recording the owner's [acceptance results](docs/ACCEPTATIETEST.md),
share the fixed [beta.10 release](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/tag/v0.2.0-beta.10)
and [tester start guide](docs/TESTER_START.md) (Dutch). Each tester uses their
own ESP32, sensors, network, MQTT broker, and Home Assistant. They report
results with [the template](docs/TESTVERSLAG_TEMPLATE.md), including items not
tested and any help needed to follow the guide. Keep codes and passwords
private; share only anonymized logs and results. The second tester's results
are still pending. The [release procedure](docs/VRIJGAVE.md) explains packaging,
fixed version tags, prereleases, and the checks before a stable 1.0.0.

## What Does This Project Do?

A gateway for up to four CTEK Battery Sense units. One ESP32-WROOM-32
(ESP32-D0WD-V3, 4 MB flash) reads them in turn and sends measurements over Wi-Fi
and MQTT to your Home Assistant instance. The protocol implementation is in
`firmware/`.

The gateway provides live voltage, current temperature, and a configurable
low-voltage warning. Historical records remain available on the gateway's MQTT
history topic. Home Assistant discovers the live sensors automatically through
MQTT Discovery. By default, each sensor is read every five minutes; readings
expire after 30 minutes. An out-of-range sensor does not prevent the others from
being read. The board's configuration page provides collapsed BLE logs and
over-the-air (OTA) updates.

The Android app does not run as an APK on the ESP32; the required communication
is implemented in C++. See the [reverse-engineering report](docs/REVERSE_ENGINEERING.md)
for analysis of the supplied CTEK Battery Sense 2.4.6 XAPK and the original
[firmware repository](https://github.com/bbartje/ctek-ble-ESP32-gateway).

This repository is the home for ongoing development and releases. The original
firmware repository is archived and remains available as a historical source,
including earlier commits, releases, and field reports. The new firmware does
not depend on it to build or run.

## Run the Workshop App

The app starts automatically in a Codespace. To start it yourself in a terminal:

```bash
npm install
npm start
```

The React workshop app runs on port 3000; it is not the ESP32 configuration page.
Run tests with `npm test` and build with `npm run build`. Tests also run the
production C++ protocol code with memory checks. Python 3 and `g++` are required;
the tests do not connect to physical sensors. Browser tests also exercise the
ESP32 interface's language selection and sensor-slot controls.

Build the firmware (requires PlatformIO Core 6.1.19):

```sh
pio run --project-dir firmware -e esp32-wroom-32
```

Build and validate the complete flash package:

```sh
python3 firmware/tools/build_release.py
npm test
```

The package includes a manifest, SHA-256 checksums, the installation guide,
tester start guide, acceptance checklist, feedback template, and release notes.
Tests also check corrupted files and whether the supplied package matches the
current firmware sources. GitHub Actions builds and validates the package after
firmware changes; see the **Actions** tab for its output.

For USB installation, use the Windows [Espressif Flash Download Tool](https://www.espressif.com/en/support/download/other-tools)
and follow the [flashing and installation guide](docs/FLASHEN.md). Flash the
complete factory image at **0x0**. The application-only image is for updates
through the gateway page; do not interchange these files.

**Beta status:** protocol and error scenarios are tested on a computer, and the
ESP32 firmware is built. No physical board or sensor is available in this
development environment. Testing with your sensors and their actual range is
still required. This version does not provide CTEK's calculated state of charge
or a complete three-month history import; see the reverse-engineering report.

## Comparison with the ESPHome Gateway

The [source comparison](docs/VERGELIJKING_ESPHOME_CTEK.md) (Dutch, October 12,
2026) records what we can learn from
[dtech77pl/esphome-ctek-battery-sense](https://github.com/dtech77pl/esphome-ctek-battery-sense):
small history batches, a persistent resume position, and separate historical
storage in Home Assistant. It also documents differences in encryption,
connection lifetime, timestamps, and cursor rollover. Full history import is
a proposed follow-up; the current 12/48-record limits still apply.

## Data and Privacy

The project handles battery voltage, temperature, sensor addresses, and
measurement times. Configure Wi-Fi/MQTT passwords and sensor codes on the board;
never add them to Git. The ESP32 stores settings locally. Home Assistant stores
measurements according to its own configuration. The protocol does not require
a CTEK cloud account. Battery data goes to your own MQTT broker; time
synchronization contacts `pool.ntp.org` and `time.cloudflare.com`, without
sending battery measurements. Vehicle names, addresses, and measurement patterns may be
private data; use fictional names in shared logs.

HTTP, MQTT, and configuration storage are unencrypted. Use a trusted home
network and do not expose the gateway or broker directly to the internet. Each
tester manages their own gateway and Home Assistant backups. Keep sensor codes
and network credentials privately so a factory reset can be configured again.

The XAPK and brand name belong to their respective rights holders. Decompiled
Android source files are not redistributed. Third-party libraries retain their
own licenses.

See the [technical firmware documentation](firmware/README.md) for MQTT topics,
history limits, dependencies, and items that still require hardware testing.

## Owner

**bbartje** owns, maintains, and is the point of contact for this personal project.
