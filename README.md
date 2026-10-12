# CTEK Battery Sense → ESP32 → Home Assistant

Owner and maintainer: **bbartje**. Firmware version: **0.2.0-beta.9**.

[Open the beta.9 release](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/tag/v0.2.0-beta.9),
download the [Wi-Fi update (.bin)](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.9/ctek-esp32-wroom-32-ota.bin),
[complete factory image (.bin, flash address 0x0)](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.9/ctek-esp32-wroom-32-factory.bin),
or [firmware package (ZIP)](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.9/ctek-esp32-wroom-32-0.2.0-beta.9.zip).
Follow the [installation guide](docs/FLASHEN.md). The ZIP includes the start
guide, acceptance checklist, feedback template, release notes, checksums,
build information, and library licenses. Earlier beta packages remain in the
repository.

**Independent testers:** begin with [the tester start guide](docs/TESTER_START.md)
(Dutch), then use [the feedback template](docs/TESTVERSLAG_TEMPLATE.md).
The owner is completing the hardware test and will confirm when the second
tester should start. The full installation guide is in English.

Beta.9 restores **Bluetooth signal strength (RSSI)** in Home Assistant and
directly on each compact sensor card, alongside **name, voltage, and last
successful measurement time**. The value is in dBm from the last successful
read; −60 dBm is stronger than −90 dBm. Missing signal strength is shown as
unavailable and an old value is labelled stale. Temperature, read status, counters,
**Read now**, logs, and sensor settings are under **Details and settings**, closed
by default. Missing live voltage is shown as unavailable; an old reading is
marked stale. Every unused slot still appears only after clicking **+**, one
at a time, as corrected in beta.7.

The gateway publishes **live voltage**, **current temperature**, **Bluetooth
signal strength**, and the low-voltage status to Home Assistant.
Historical measurements remain available on the
gateway's history topic, but are no longer separate Home Assistant sensor
entities. The device is identified as **BartjeB**. The web interface supports
Dutch, English, and German, remembers the selected language, and displays the
last successful read in the selected local time zone. A two-minute recovery
access point is available after startup; existing settings are not erased.
The public recovery login works only on that setup network and only to choose
a new admin password. It cannot open configuration, diagnostics, or firmware
uploads. Anyone nearby during the window can still reset the password; use
recovery in a private setting. Normal network access always requires your own
admin password.

**Current status (October 12, 2026):** beta.9 is published as a GitHub prerelease
with a fixed version tag and verified files. The firmware binaries match those
already provided for the owner's test; this update completes the documentation
and tester package. The owner is starting the hardware test; installation and
the complete acceptance results have not yet been confirmed. The second
tester's instructions are ready and their results are still pending.

For an overview, read the [beta.9 release notes](docs/RELEASE_0.2.0-beta.9.md).
The [quality report for beta.6](KWALITEITSRAPPORT-2026-10-12.md) and
[test status and next steps](docs/VERVOLGTEST.md) are in Dutch and record the
software checks, known limitations, and outstanding hardware tests. Use the
[acceptance checklist](docs/ACCEPTATIETEST.md) before inviting external testers.
Keep the beta label during external testing; release 1.0.0 only after
installation, repeated reads, Home Assistant, recovery, and updates have passed
on testers' own devices.

On October 12, 2026, the first complete physical read was confirmed for **sensor
slot 3 on beta.4**: connection, unlock, metadata reads, and receipt of two history
records all succeeded. The admin page displayed voltage and temperature, and
this confirmed the saved code for that sensor. Voltage and temperature were
also reported in Home Assistant. Repeated reads, the other sensor slots, and
the complete beta.9 acceptance checklist still need physical confirmation.

**PlatformIO and PuTTY are not required to install this firmware.** For the
first factory flash, connect to `CTEK-Setup-xxxxxx` with the initial password
**CTEK-Start123**. Open **http://192.168.4.1**, sign in as **admin** with the same
initial password, and choose a personal password. The board restarts; reconnect
and sign in with your new password. You can then configure Wi-Fi, MQTT, and
sensors. A Wi-Fi update preserves your existing admin password; a factory flash
erases the previous settings.

## Update an Existing Gateway (OTA)

Download the [beta.9 OTA image](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.9/ctek-esp32-wroom-32-ota.bin).
On the gateway page, choose **Pause polling and update firmware**, wait for any
active sensor read to finish, then upload `ctek-esp32-wroom-32-ota.bin`. Keep
the ESP32 powered until it restarts. Wi-Fi/MQTT settings, sensor configuration,
and your admin password are preserved. After restarting, check the version is
**0.2.0-beta.9**. Each configured card should show name, voltage, Bluetooth
signal, and measurement time, with **Details and settings** closed. After a
successful reading, check **Bluetooth-signaal** under the device's sensors in
Home Assistant. Empty sensor forms
appear only after clicking **+**.
Do not upload the factory image through
this page. See [the complete OTA instructions](docs/FLASHEN.md#5-update-firmware-over-wi-fi)
for update checks, recovery, and troubleshooting.

## Start a Second Hardware Test

After recording the owner's results, share the fixed
[beta.9 release link](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/tag/v0.2.0-beta.9)
and [tester start guide](docs/TESTER_START.md). Each tester uses their own
ESP32, sensors, network, MQTT broker, and Home Assistant. They report results
with [the template](docs/TESTVERSLAG_TEMPLATE.md), including items not tested
and any help needed to follow the guide. Keep codes and passwords private;
share only anonymized logs and results.

The [release procedure](docs/VRIJGAVE.md) explains package checks, version tags,
prereleases, hardware acceptance, and the steps before a stable 1.0.0.

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
