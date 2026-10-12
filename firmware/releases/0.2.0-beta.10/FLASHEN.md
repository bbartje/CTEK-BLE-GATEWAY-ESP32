# Flash and Set Up the Gateway Without PlatformIO or PuTTY

For **ESP32-WROOM-32, ESP32-D0WD-V3 revision 3, with 4 MB flash** only.
Firmware **0.2.0-beta.10** is a published prerelease and still needs testing
with your physical Battery Sense units. This image is not for ESP32-C3/S3/S2.

For a first independent test, begin with the
[tester start guide](TESTER_START.md) (Dutch) and keep a copy of the
[feedback template](TESTVERSLAG_TEMPLATE.md). Download the
[beta.10 package](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.10/ctek-esp32-wroom-32-0.2.0-beta.10.zip)
from the fixed [beta.10 release](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/tag/v0.2.0-beta.10).
Earlier releases remain available with their original files.

## 1. First Flash with the ESP32 Flash Download Tool

Download the official [Espressif Flash Download Tool](https://www.espressif.com/en/support/download/other-tools).
Download the [complete factory image](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.10/ctek-esp32-wroom-32-factory.bin)
or extract the ZIP from `firmware/releases/`. For USB installation, use
**ctek-esp32-wroom-32-factory.bin**. PlatformIO is not needed to flash it.

1. Connect the board with a USB data cable. In Espressif's tool, select
   **ChipType ESP32**, **WorkMode Develop**, and **LoadMode UART**.
2. Select the factory image, enable only that row, and enter address **0x0**.
   **0x00000000** is also valid.
3. Where available, select **DIO**, **40 MHz**, and **32 Mbit / 4 MB**. Choose
   **DoNotChgBin** to preserve the validated image header. Recent tool versions
   read these settings from the image.
4. Select your COM port and start at **115200 baud**. Click **START**. If the
   connection fails, hold **BOOT**, briefly press **EN/RESET**, start the
   download, then release BOOT when flashing begins.
5. Wait for **FINISH** and reset the board. A factory flash also erases the
   configuration area, so use it for first installation or recovery. An erase
   is normally unnecessary and will remove all existing configuration.

The factory image already combines the bootloader at 0x1000, partition table at
0x8000, OTA start data at 0xE000, and application at 0x10000. Do not add separate
rows alongside the factory image. Flashing an application-only image at 0x0 is
incorrect.

## 2. Configure over Wi-Fi; PuTTY Is Not Needed

After a factory flash of this version, the initial login details are:

| Item | Value |
|---|---|
| Wi-Fi network | `CTEK-Setup-xxxxxx` |
| Initial Wi-Fi password | **CTEK-Start123** |
| Configuration address | **http://192.168.4.1** |
| Username | **admin** |
| Initial admin password | **CTEK-Start123** |

1. Keep the board powered and press EN/RESET after flashing.
2. Connect your phone or laptop to `CTEK-Setup-xxxxxx` with password
   **CTEK-Start123**. Your phone may try to leave the network because it has no
   internet access; choose to stay connected.
3. Open **http://192.168.4.1**. If prompted, sign in as **admin** with
   **CTEK-Start123**.
4. Select **Choose your own password first**. Choose 12–63 characters using
   letters, numbers, and ordinary punctuation, without spaces or accents. Enter
   it twice and save. Do not use the initial password as your own. Wi-Fi, MQTT,
   sensors, and firmware upload remain blocked until this step is complete.
5. The board restarts and the setup network briefly disappears. Reconnect to
   `CTEK-Setup-xxxxxx` using **your new password**. If your phone or laptop still
   uses the initial password, forget the network in its Wi-Fi settings and
   connect again.
6. Open **http://192.168.4.1** again and sign in as **admin** with your new
   password. Your browser may still remember the old login and ask you to enter
   the correct credentials again. Keep your password private.
7. Enter your **2.4 GHz Wi-Fi** details and existing MQTT broker. Use the broker
   address, such as that of a Mosquitto add-on, not a Home Assistant URL. Enter
   the MQTT username and password; keep port 1883 if that matches your broker.
   Choose your local time zone in the same form. The default is Europe/Amsterdam;
   the listed regions apply daylight-saving changes automatically. Save the settings.
   The gateway restarts.
8. Reconnect your laptop to your regular network. Open `http://ctek-xxxxxx.local`
   if your network supports it (`xxxxxx` is the same suffix as the setup network).
   Otherwise, find the device's IP address in your router's connected-device list;
   its name is `ctek-xxxxxx`. Reserving this IP in the router is recommended. If
   Wi-Fi does not connect, the setup network returns after about one minute,
   using **your own password**.

On every boot with saved Wi-Fi settings, the gateway also opens `CTEK-Setup-xxxxxx`
for a **two-minute recovery window**, using **CTEK-Start123**. Normal Wi-Fi,
BLE polling, and MQTT startup continue immediately; this window does not reset
or change saved settings. To recover a forgotten password:

1. Restart the board and connect to `CTEK-Setup-xxxxxx` with the initial Wi-Fi
   password within two minutes.
2. Open **http://192.168.4.1**, sign in as **admin** with the initial password,
   and enter your new admin password twice. This login only opens password
   recovery; configuration, diagnostics, and firmware upload remain blocked.
3. Save before the two-minute deadline. The board restarts with Wi-Fi, MQTT,
   and sensors preserved. Reconnect to your normal network and sign in to the
   gateway with the new admin password. If home Wi-Fi cannot connect, wait for
   the private setup network to return after the recovery window ends.

The initial password is rejected on the normal network. The public recovery
login closes after two minutes even if a device is still connected.
If the saved Wi-Fi is still unavailable later, the existing setup access point
may return using your private admin password. The initial first-time password
setup remains available until you choose a private password. Because the
recovery password is public, anyone nearby during the window can reset the
admin password. Perform setup and recovery in a private setting. HTTP and MQTT
are unencrypted; keep the gateway on a trusted home network, without internet
port forwarding. Keep your sensor codes and network credentials privately, and
back up Home Assistant before testing updates.

An OTA update from an earlier version preserves your existing admin password;
it does not restore the initial password. If you cannot recover your old
password, install this new factory image at **0x0**. This erases the old settings
and restarts the setup process above.

## 3. Test One Sensor, Then Add the Other Three

1. Fully close the CTEK app to release any existing Bluetooth connection. Start
   with one sensor close to the ESP32.
2. Click **Search for Bluetooth devices**, wait six seconds, and refresh the
   page. Click **+** to open slot 1, then copy the address and address type.
   The scan lists candidates;
   the actual CTEK service is verified when connecting.
3. Enter a name and the code from the sensor label: 16 characters, or the 11
   characters without the `40149` prefix. Save. You can correct an invalid code
   on the web page; open **Details and settings** to edit a saved sensor.
4. Refresh after 15–60 seconds. The compact sensor card shows its name,
   voltage, Bluetooth signal strength, and the last successful measurement time.
   The Bluetooth value is from the last successful read, in dBm; −60 dBm is
   stronger than −90 dBm. Open **Details and
   settings** to check temperature and **Reading received**. A new sensor
   without history uses live voltage. With the vehicle stationary, compare
   voltage with the CTEK app or a multimeter, then close the app again.
5. Confirm the MQTT integration is active in Home Assistant. Under
   **Settings → Devices & services → MQTT**, the device should appear with live
   voltage, current temperature, **Bluetooth-signaal** in dBm, and **Low voltage**
   status. Home Assistant
   stores its own history from these live updates. The default display uses two
   decimal places for voltage and one for temperature. A precision setting you
   chose manually in Home Assistant takes precedence.
6. Save the first sensor, then use the **+** control to reveal another slot.
   Saved sensors stay visible; every empty slot starts hidden, including slot 4
   when three sensors are configured. Each click reveals one empty slot. Add
   sensors 2–4 one at a time. Over at least two polling rounds, confirm that each sensor
   has more than one successful read. The default interval is five minutes per
   sensor; the ESP32 does not keep four connections open continuously.

A failure does not produce a zero reading. Old values become unavailable after
30 minutes. On the card, an old voltage is marked **stale**. Missing live voltage
is shown as **Unavailable**; the details can show an explicitly labelled
historical fallback. Open **Details and settings** and use **Read now** to
request an earlier retry. **Download diagnostics**
includes procedure details and raw measurement metadata, but no codes or
passwords. Remove sensor addresses before sharing diagnostics publicly. USB is
not needed for these checks.

Bluetooth signal strength also becomes stale with old readings. Before the
first successful read or if RSSI cannot be measured, the card says
**Unavailable** and Home Assistant has no numeric RSSI value.

## 4. Check the Final Installation Location

Move the board only after sensor 1 and MQTT work. At the final location, test
with vehicle hoods or garage doors in their normal positions. Check Wi-Fi and
all four BLE signals for several polling rounds. Distance alone is not a useful
guide because metal and interference affect radio range. Use a stable USB power
supply; this project does not connect the ESP32 directly to 12 V.

Optionally, make one sensor unreachable and confirm the others continue to
update. After 30 minutes, that sensor should become unavailable. Bring it back
within range and confirm that it recovers without restarting the board.

## 5. Update Firmware over Wi-Fi

For an update from beta.1 through beta.9, [download the beta.10 OTA image](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.10/ctek-esp32-wroom-32-ota.bin).

1. Open the gateway page and select **Update firmware** (older versions:
   **Pause polling and update firmware**). Wait
   for the active read to finish, then refresh the update page.
2. Select only **ctek-esp32-wroom-32-ota.bin** from the new package. The factory
   image is rejected here. Upload the file and keep the board powered.
3. After validation, the gateway restarts. Check the version, sensors, and
   successful readings. Settings and passwords are preserved.
4. If an upload is cancelled or fails, the current firmware remains selected.
   Polling resumes after **Cancel** or five minutes of inactivity.

After the update, confirm the page reports **0.2.0-beta.10**. **Gateway settings**
starts closed; open it to change Wi-Fi/MQTT or other gateway settings.
The toolbar stays at the top while scrolling. Check it on a phone as well.
Each configured
sensor card shows name, voltage, Bluetooth signal strength, and the last
successful measurement time.
**Details and settings** starts closed and contains temperature, status,
counters, **Read now**, logs, and the settings form. Empty sensor forms appear
only after clicking **+**. Refreshing the page closes the details and hides any
empty forms that have not been saved. The gateway
republishes Discovery: voltage defaults to two decimal places and current
temperature to one. The `voltage`, `temperature`, and `low_voltage` entity IDs
remain stable. **Bluetooth-signaal** is restored using its original RSSI
entity identity and appears under the regular sensors. After a successful read,
compare its dBm value with the gateway card. The old separate
`current_temperature` entity is removed; the existing `temperature` entity now represents the
current reading. Home Assistant continues recording its own history. The
device manufacturer is shown as **BartjeB**. Your own display settings take
precedence; you do not need to remove and re-add the device.

If a live voltage read is missing, its Home Assistant value and the low-voltage
signal become **unknown**, rather than reporting a historical value as current.
The gateway page labels any historical fallback explicitly. Treat unknown or
unavailable data as missing information; it does not confirm a healthy battery.

For a sensor, open **Details and settings**, click **Read now**, and inspect
the **BLE log**. `GATT-discovery`
shows the number of discovered characteristics, their UUIDs, and capabilities.
A missing characteristic, one with `read=0`, and a failed read have different
messages. `unlock write: OK` means only that the message was sent; it does not
confirm the code. On a connection failure, the code was not sent. Beta.4 adds
`BLE-disconnect` with the reason, read phase, and timing for a disconnect before
cleanup. This reason alone does not prove the code is wrong. While the status
shows **BLE read in progress**, the log still belongs to the previous completed
attempt; wait for the read to finish and refresh the page. If reads still fail,
download the latest diagnostics and share them without addresses or other
private information.

A successful upload does not prove that new program logic works correctly.
There is no automatic boot rollback. USB recovery may still be needed if a later
version will not start or Wi-Fi stops working.

## 6. Create a Home Assistant Warning

In **Settings → Automations**, create an automation for the discovered
**Low voltage** signal. For example, when its state is **on** for 15 minutes,
send a notification to your phone. Choose your own gateway threshold; 12.2 V is
only the initial setting, not a universal battery diagnosis. Also create a
notification for a sensor that remains unavailable, so missing data is not
mistaken for a healthy battery.

This version does not provide an exact CTEK state-of-charge percentage. The
standard Home Assistant graph starts when measurements are received; old records
on `/history` are not automatically backfilled into the graph. See the
[firmware documentation](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/blob/main/firmware/README.md) for more information.

Before inviting external testers, complete the owner's
[acceptance checklist](ACCEPTATIETEST.md). Keep the beta label during external testing;
version 1.0.0 follows successful testing on their own hardware and networks.
