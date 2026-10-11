# ESP32 CTEK-gateway 0.2.0-beta.1

Firmware voor ESP32-WROOM-32 / ESP32-D0WD-V3 met 4 MB flash. Maximaal vier
CTEK Battery Sense-units worden om de beurt uitgelezen en via MQTT naar
Home Assistant gestuurd.

- Spanning, actuele en historische temperatuur, signaalsterkte en instelbare
  melding voor lage spanning; automatische Home Assistant MQTT Discovery.
- Instellingen en diagnostiek via de webpagina op het bord.
- Updates via wifi, met behoud van instellingen.
- Onderzoek van de Android-app, geteste protocolkern en gecontroleerde
  flashbestanden met SHA-256-controlesommen.

Download bij **Assets** bij voorkeur `ctek-esp32-wroom-32-0.2.0-beta.1.zip`.
Het pakket bevat alle bestanden, handleiding, bouwgegevens en licenties.

Voor de eerste installatie met **ESP32 Flash Download Tool**:

1. Pak de ZIP uit en kies `ctek-esp32-wroom-32-factory.bin`.
2. Gebruik adres **0x0**, chip **ESP32**, **DIO**, **40 MHz**, **4 MB**.
3. Volg [de installatiehandleiding](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/blob/v0.2.0-beta.1/docs/FLASHEN.md),
   inclusief het uitlezen van het beheerwachtwoord via USB-serieel.

Voor een volgende update via de webpagina gebruik je alleen
`ctek-esp32-wroom-32-ota.bin`. De factory-bin wist de opgeslagen instellingen;
de OTA-bin behoudt ze.

De firmwarebuild, lokale tests, controles van de echte binaries en
[GitHub Actions-controle](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/actions/runs/38163352675)
zijn geslaagd. Zie [het onderzoeksrapport](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/blob/v0.2.0-beta.1/docs/REVERSE_ENGINEERING.md)
voor bewijs en grenzen.

**Bèta:** deze firmware is nog niet op een fysiek bord met sensoren getest.
Begin met één sensor; controleer daarna MQTT, bereik, vier sensoren en een
wifi-update volgens de handleiding. Een exact CTEK-laadpercentage en volledige
import van drie maanden historie zijn niet opgenomen. Home Assistant bewaart
de ontvangen metingen; MQTT is geen gegarandeerd archief van iedere meting.

Deze repository is de plek voor verder onderhoud. De oude
[ctek-ble-ESP32-gateway](https://github.com/bbartje/ctek-ble-ESP32-gateway)
is al gearchiveerd en blijft beschikbaar als historische bron.
