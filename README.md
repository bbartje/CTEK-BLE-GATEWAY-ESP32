# CTEK Battery Sense → ESP32 → Home Assistant

> **Workshop-deelnemer?** Maak eerst je eigen kopie: klik op **Use this template → Create a new repository**, geef hem een naam en kies **Private**. Open daarna in je nieuwe repository **Code → Codespaces → Create codespace on main**. In de Codespace vind je de uitleg in [WORKSHOP.md](WORKSHOP.md).

## Wat doet deze app?

Een gateway voor maximaal vier CTEK Battery Sense-units, om de beurt uitgelezen
met één ESP32-WROOM-32 (ESP32-D0WD-V3, 4 MB flash). Metingen gaan via wifi en MQTT
naar de eigen Home Assistant. De protocolkern staat in `firmware/`.

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

De protocolkern is toegevoegd en getest. De ESP32-runtime en het complete
flashpakket worden daarop aangesloten. Een softwaretest bewijst nog geen
werkende radioverbinding met jouw sensor.

## Gegevens

Batterijspanning, temperatuur, sensoradressen en meettijden. Wifi-/MQTT-wachtwoorden
en verbindingscodes worden op het bord ingesteld en horen nooit in Git.
De ESP32 bewaart instellingen lokaal; Home Assistant bewaart de metingen volgens
de eigen instellingen. Er is geen CTEK-cloudaccount nodig voor het gevonden protocol.
Voertuignamen, adressen en meetpatronen kunnen privégegevens worden wanneer je
ze deelt; gebruik fictieve namen in gedeelde logs.

De XAPK en merknaam zijn van hun respectieve rechthebbenden. Gedecompileerde
Android-bronbestanden worden niet opnieuw gepubliceerd. De gebruikte
bibliotheken houden hun eigen licenties.

## Eigenaar

**bbartje** is eigenaar, aanspreekpunt en beheerder van dit persoonlijke project.
