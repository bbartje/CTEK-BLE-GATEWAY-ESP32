# CTEK Battery Sense → ESP32 → Home Assistant

Eigenaar en beheerder: **bbartje**. Firmwareversie: **0.2.0-beta.3**.

[Download de wifi-update (.bin)](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.3/ctek-esp32-wroom-32-ota.bin),
[de volledige flashfirmware (.bin, adres 0x0)](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.3/ctek-esp32-wroom-32-factory.bin)
of [het flashpakket (ZIP, circa 1,5 MB)](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.3/ctek-esp32-wroom-32-0.2.0-beta.3.zip)
en volg de [installatiehandleiding](docs/FLASHEN.md).
De [bètarelease 0.2.0-beta.3](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/tag/v0.2.0-beta.3)
bevat ook de losse flashbestanden, controlesommen en bouwgegevens.

Beta.3 zoekt de Bluetooth-leespunten daadwerkelijk op voordat de uitlezing
begint. De BLE-log toont hun lees-/schrijfmogelijkheden en maakt onderscheid
tussen een ontbrekend leespunt, ontbrekende leestoegang en een mislukte read.
Een verstuurd ontgrendelbericht bevestigt de code nog niet. Bij een mislukte
Bluetooth-verbinding meldt de gateway dat de code niet is verstuurd; een
verbindingsstatus wordt niet meer als readstatus gepresenteerd. Deze wijziging
moet nog met de echte sensoren worden getest; hij garandeert geen geslaagde
uitlezing. Zie [de releasenotities](docs/RELEASE_0.2.0-beta.3.md).

**PlatformIO en PuTTY zijn niet nodig om deze firmware te installeren.** Na de
eerste factory-flash verbind je met `CTEK-Setup-xxxxxx`, met startwachtwoord
**CTEK-Start123**. Open **http://192.168.4.1**, log in als **admin** met hetzelfde
startwachtwoord en kies verplicht een eigen wachtwoord. Daarna herstart het
bord; verbind en log opnieuw in met je nieuwe wachtwoord. Pas dan kun je wifi,
MQTT en sensoren instellen. Een update via wifi behoudt een bestaand eigen
beheerwachtwoord; de factory-flash wist de eerdere instellingen.

## Wat doet deze app?

Een gateway voor maximaal vier CTEK Battery Sense-units, om de beurt uitgelezen
met één ESP32-WROOM-32 (ESP32-D0WD-V3, 4 MB flash). Metingen gaan via wifi en MQTT
naar de eigen Home Assistant. De protocolkern staat in `firmware/`.

De gateway biedt spanning, historische en actuele temperatuur, Bluetooth-
signaalsterkte en een instelbare melding voor lage spanning. Home Assistant
ontdekt de sensoren automatisch met MQTT Discovery. Iedere sensor wordt
standaard eens per vijf minuten uitgelezen; na 30 minuten vervallen oude
metingen. Een sensor buiten bereik verhindert het uitlezen van de andere niet.
De configuratiepagina op het bord biedt logs en updates via wifi (OTA).

De Android-app draait niet als APK op een ESP32: de benodigde communicatie wordt
in C++ uitgevoerd. Zie [het onderzoeksrapport](docs/REVERSE_ENGINEERING.md) voor
de analyse van de meegeleverde CTEK Battery Sense 2.4.6 XAPK en de bestaande
[firmware-repository](https://github.com/bbartje/ctek-ble-ESP32-gateway).

Deze repository is de plek voor verdere ontwikkeling en releases. De oude
firmware-repository is gearchiveerd en blijft beschikbaar als historische bron,
inclusief eerdere commits, releases en praktijkmeldingen. De nieuwe firmware
heeft die repository niet nodig om te bouwen of te draaien.

## Hoe start je de app?

In een Codespace start de app vanzelf. Zelf starten in een terminal:

```bash
npm install
npm start
```

De React-workshopapp draait dan op poort 3000; dit is niet de configuratiepagina
van de ESP32. Tests uitvoeren doe je met `npm test`; bouwen met `npm run build`.
De tests voeren ook de echte C++-protocolkern uit met geheugencontroles.
Daarvoor zijn Python 3 en `g++` nodig. Er worden geen fysieke sensoren benaderd.

Firmware bouwen (PlatformIO Core 6.1.19 nodig):

```sh
pio run --project-dir firmware -e esp32-wroom-32
```

Een compleet flashpakket opnieuw maken en controleren:

```sh
python3 firmware/tools/build_release.py
npm test
```

Het pakket bevat een manifest, SHA-256-controlesommen en de handleiding. De tests
controleren ook corrupte bestanden en of de huidige firmwarebronnen overeenkomen
met het geleverde pakket. GitHub Actions bouwt en controleert eveneens het pakket
bij firmwarewijzigingen; de uitvoer staat onder de tab **Actions**.

Gebruik de [flash- en installatiehandleiding](docs/FLASHEN.md) voor het Windows
ESP32 Flash Download Tool. De complete factory-bin hoort op **0x0**; de losse
applicatie-bin is voor updates via de gatewaypagina. Wissel die bestanden niet om.

**Bèta:** de protocol- en foutscenario's worden op de computer getest en de
echte ESP32-firmware wordt gebouwd. Er is geen fysiek bord/sensor beschikbaar
in deze omgeving. Eén test met jouw sensoren en hun bereik blijft noodzakelijk.
Een identiek CTEK-laadpercentage en volledige import van drie maanden historie
zitten niet in deze versie; zie het onderzoeksrapport.

## Gegevens

Batterijspanning, temperatuur, sensoradressen en meettijden. Wifi-/MQTT-wachtwoorden
en verbindingscodes worden op het bord ingesteld en horen nooit in Git.
De ESP32 bewaart instellingen lokaal; Home Assistant bewaart de metingen volgens
de eigen instellingen. Er is geen CTEK-cloudaccount nodig voor het gevonden protocol.
Het bekende startwachtwoord werkt alleen tijdens de eerste setup; de webpagina
blokkeert normaal beheer totdat je een eigen wachtwoord hebt opgeslagen.
Daarna is dit eigen wachtwoord ook het setupwifi-wachtwoord. Instellingen staan
in het niet-versleutelde NVS-
flashgeheugen en worden bij OTA behouden. De verbindingen zijn HTTP en MQTT
zonder TLS; gebruik het eigen vertrouwde netwerk. Tijdsynchronisatie gebruikt
`pool.ntp.org` en `time.cloudflare.com`. Bij ontbreken van tijd worden geen
verzonnen kalenderdatums gepubliceerd.
Voertuignamen, adressen en meetpatronen kunnen privégegevens worden wanneer je
ze deelt; gebruik fictieve namen in gedeelde logs.

De XAPK en merknaam zijn van hun respectieve rechthebbenden. Gedecompileerde
Android-bronbestanden worden niet opnieuw gepubliceerd. De gebruikte
bibliotheken houden hun eigen licenties.

Zie [technische informatie](firmware/README.md) voor onderwerpen, historiegrenzen,
bibliotheken en wat nog op hardware moet worden bevestigd.

## Eigenaar

**bbartje** is eigenaar, aanspreekpunt en beheerder van dit persoonlijke project.
