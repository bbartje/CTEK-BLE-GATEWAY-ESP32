# Firmware dependencies en bronnen

De gateway-broncode en bouwinstructies staan in
[bbartje/CTEK-BLE-GATEWAY-ESP32](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32).
De binaire firmware bevat onderstaande componenten; hun eigen licenties blijven
van toepassing. De upstream sources horen bij de vastgezette versies in
`platformio.ini` en kunnen met PlatformIO opnieuw worden opgehaald en gelinkt.

| Component | Versie | Bron / licentie |
|---|---|---|
| Arduino ESP32 core | 2.0.17, package 4.20017.260907+sha.dcc1105b | [Espressif Arduino ESP32](https://github.com/espressif/arduino-esp32/tree/2.0.17), LGPL-2.1-or-later |
| ESP-IDF en voorgebouwde SDK | 4.4.7, gebundeld bij Arduino-package | [ESP-IDF](https://github.com/espressif/esp-idf/tree/v4.4.7), componentgebonden licenties, grotendeels Apache-2.0 |
| PlatformIO platform | 7.1.3 | [platform-espressif32](https://github.com/platformio/platform-espressif32), Apache-2.0 |
| NimBLE-Arduino | 2.5.1 | [h2zero/NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino), Apache-2.0; NimBLE/tinycrypt-licenties meegeleverd |
| PubSubClient | 2.8.0 | [knolleary/pubsubclient](https://github.com/knolleary/pubsubclient), MIT |
| ArduinoJson | 7.4.3 | [bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson), MIT |
| libb64 | Arduino-bundel | BSD-achtige tekst in `licenses/libb64.txt` |

Een exemplaar van de direct meegeleverde librarylicenties staat in het
flashpakket onder `licenses/`. De SDK bevat ook andere componentgebonden
copyrights en licenties; raadpleeg de upstream bronnen en package voor de
volledige teksten. De ESP32 ROM en sommige radio-bibliotheken horen bij het
Espressif-platform. De XAPK is alleen analyse-input en zit niet in het flashpakket.

De Arduino WebServer-parser krijgt tijdens bouwen een gerichte beveiligings-
correctie voor CVE-2026-42854. De oorspronkelijke copyrights en LGPL-licentie
blijven gelden. De reproduceerbare wijziging staat in
[`tools/webserver_security.py`](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/blob/main/firmware/tools/webserver_security.py); de SDK-installatie
wordt niet gewijzigd.

De communicatie is onafhankelijk opnieuw geïmplementeerd op basis van
vastgestelde protocolfeiten. Gedecompileerde Android-bronnen zijn niet
gekopieerd in de runtime. CTEK-merken en appmateriaal blijven van CTEK en hun
respectieve rechthebbenden. Dit is geen officiële CTEK-firmware.
