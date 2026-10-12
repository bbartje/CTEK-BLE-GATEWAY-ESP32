# Start hier: CTEK-gateway testen

Bètarelease **0.2.0-beta.10** · Eigenaar en aanspreekpunt: **bbartje**.

Dit is een beperkte bètatest met een eigen ESP32, CTEK-sensor en thuisnetwerk.
De softwarecontroles zijn geslaagd. De volledige praktijktest van de eigenaar
is nog niet bevestigd; de eigenaar meldt wanneer de tweede tester kan starten.
Noteer ook wat niet werkt of nog niet getest is.

## Wat doet de gateway?

De ESP32 leest maximaal vier CTEK Battery Sense-sensoren om de beurt uit.
Hij stuurt spanning, temperatuur, Bluetooth-ontvangststerkte en de
lage-spanningsstatus naar je eigen Home Assistant via MQTT. MQTT is de
berichtenverbinding tussen de gateway en Home Assistant.

Op de gateway zie je per sensor naam, spanning, Bluetooth-ontvangst en de
laatste meettijd. Temperatuur, instellingen, logboeken en `Nu uitlezen` staan
onder `Details en instellingen`. Een lege sensorplek open je met `+`.
Wifi/MQTT en andere gatewayinstellingen open je via `Gateway-instellingen`.
De actieknoppen blijven bij scrollen bovenaan bereikbaar. Beta.10 heeft een
retrostijl en een link naar onze repository.

## Wat heb je nodig?

- Een **ESP32-WROOM-32 / ESP32-D0WD-V3 met 4 MB flash**. De bestanden zijn
  voor dit bord; ESP32-C3, S2 en S3 hebben andere firmware nodig.
- Een stabiele USB-voeding en, voor de eerste installatie, een USB-datakabel.
- Minimaal één CTEK Battery Sense-sensor met de eigen code op het label.
- Een 2,4 GHz-wifinetwerk en je eigen Home Assistant met werkende MQTT-broker.
- Voor de beschreven eerste USB-installatie: een Windows-computer met de
  Espressif Flash Download Tool uit de [installatiehandleiding](FLASHEN.md).

Bewaar codes en wachtwoorden privé. Maak vóór de test een back-up van Home
Assistant. Gebruik een vertrouwd thuisnetwerk: HTTP, MQTT en de opgeslagen
instellingen zijn niet versleuteld. Tijdens twee minuten na opstart kan iemand
in de buurt via herstel-wifi het beheerwachtwoord vervangen. De volledige
herstelprocedure staat in [FLASHEN.md](FLASHEN.md).

## Kies de juiste download

Het [complete beta.10-pakket](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.10/ctek-esp32-wroom-32-0.2.0-beta.10.zip)
bevat de binaries, deze instructies, de controles en een leeg testverslag.
Download dit uit de vaste [beta.10-bètarelease](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/tag/v0.2.0-beta.10).
Daar vind je ook de losse [OTA-bin voor wifi-updates](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.10/ctek-esp32-wroom-32-ota.bin)
en [factory-bin voor USB](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.10/ctek-esp32-wroom-32-factory.bin).
Oudere releases en de bijbehorende pakketten blijven beschikbaar.

| Jouw situatie | Bestand | Installatie |
|---|---|---|
| Nieuw bord of schone installatie | `ctek-esp32-wroom-32-factory.bin` | USB, flashadres **0x0**; wist bestaande instellingen. |
| Gateway met beta.1–beta.9 bijwerken | `ctek-esp32-wroom-32-ota.bin` | Upload via de updatepagina van de gateway; behoudt instellingen en wachtwoord. |
| Alles samen downloaden | `ctek-esp32-wroom-32-0.2.0-beta.10.zip` | Pak uit en kies daarna het juiste bestand hierboven. |

De losse binaries hebben bij iedere versie dezelfde bestandsnaam. Download
ze daarom uit het **beta.10-pakket** en controleer na installatie dat de
beheerpagina **0.2.0-beta.10** toont.

## Begin met één sensor

1. Volg [FLASHEN.md](FLASHEN.md) voor USB-installatie of OTA. PlatformIO en
   PuTTY zijn hiervoor niet nodig. Kies bij een nieuwe installatie eerst
   een eigen beheerwachtwoord en open daarna `Gateway-instellingen` om wifi
   en MQTT in te stellen.
2. Zet de ESP32 dichtbij één sensor. Sluit de CTEK-app en zet
   telefoon-Bluetooth uit tijdens de gatewaytest.
3. Zoek Bluetooth-apparaten op de gateway en gebruik het adres en adrestype
   uit de scan. Open een lege plek met `+`, geef een fictieve testnaam en
   vul de code van juist die sensor in. Bewaar de code alleen op je gateway.
4. Controleer na een afgeronde uitlezing spanning, meettijd en Bluetooth-
   ontvangst. Open de details voor temperatuur en `Meting ontvangen`.
   −60 dBm is sterker dan −90 dBm; de ontvangstwaarde hoort bij de laatste
   geslaagde uitlezing. Een ontbrekende waarde wordt als niet beschikbaar gemeld.
5. Controleer bij het MQTT-apparaat in Home Assistant spanning, temperatuur,
   `Bluetooth-signaal` en lage-spanningsstatus. Laat meerdere automatische
   rondes lopen: standaard wordt iedere sensor om de vijf minuten uitgelezen.
6. Voeg andere sensoren één voor één toe. Test daarna ook de uiteindelijke
   plaatsing, herstarts, wifi-/MQTT-herstel, wachtwoordherstel en OTA met de
   [acceptatiechecklist](ACCEPTATIETEST.md). Laat minimaal 24 uur draaien.

Home Assistant bewaart de historie vanaf ontvangst van de metingen. De
gateway toont geen berekend CTEK-laadpercentage. Bij uitval worden gegevens
na 30 minuten niet meer als vers beschouwd. USB-herstel kan nodig zijn als
een update niet opstart; er is geen automatische terugval na een mislukte boot.

## Geef je uitslag door

Maak voor jezelf een kopie van [TESTVERSLAG_TEMPLATE.md](TESTVERSLAG_TEMPLATE.md).
Gebruik **geslaagd**, **mislukt** of **niet getest** bij iedere controle.
Noteer bordtype, firmwareversie, Home Assistant-versie, aantal sensoren,
testduur en het foutverloop. Vermeld of je dit zelfstandig met de handleiding
kon uitvoeren.

Deel de uitslag met bbartje via de afgesproken route. Controleer een eventuele
diagnostiekdownload vóór het delen: verwijder echte Bluetooth-adressen,
voertuignamen en andere privégegevens. Deel geen codes, wifi-/MQTT-wachtwoorden
of beheerwachtwoorden. Een schermafbeelding mag die gegevens ook niet bevatten.

De eigenaar verzamelt de resultaten en legt het besluit vast. Een tweede
tester bewijst alleen de scenario's die daadwerkelijk op diens eigen hardware
zijn uitgevoerd; een stabiele versie 1.0.0 volgt na geslaagde externe tests.
