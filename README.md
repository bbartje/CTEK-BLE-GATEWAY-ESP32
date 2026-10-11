# CTEK Battery Sense → ESP32 → Home Assistant

Eigenaar en beheerder: **bbartje**. Firmwareversie: **0.2.0-beta.1**.

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
Bij de eerste start genereert het bord een eigen beheerwachtwoord; dit lees je
op USB-serieel op 115200 baud. Instellingen staan in het niet-versleutelde NVS-
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
