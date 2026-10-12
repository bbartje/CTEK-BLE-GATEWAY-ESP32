# Teststatus en verdergaan

Bijgewerkt op 12 oktober 2026. Eigenaar en beheerder: bbartje.

## Opgeslagen firmware

- Nieuwe diagnoseversie: **0.2.0-beta.4**, voor ESP32-WROOM-32 met 4 MB flash.
  Bron en flashbestanden staan in deze repository; de eerdere pakketten blijven.
- De eerdere beta.3-firmware en flashbestanden zijn gecommit en gepusht in
  commit `8db6863f0194155364d02ce2601a1a3342cab8cb`.
- [De publieke bètarelease](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/tag/v0.2.0-beta.3)
  is met expliciete toestemming van de gebruiker gepubliceerd.
- [De OTA-bin voor een wifi-update](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.3/ctek-esp32-wroom-32-ota.bin?download=1)
  is 1.181.392 bytes. Grootte en SHA-256 van de publieke download zijn
  gecontroleerd tegen de lokale binary.
- De nieuwe bestanden staan in `firmware/releases/0.2.0-beta.4/`; de ZIP staat
  één map hoger. Zie [de releasenotities](RELEASE_0.2.0-beta.4.md).

Beta.3 zoekt alle GATT-punten daadwerkelijk op, logt hun mogelijkheden en
maakt onderscheid tussen ontbrekende punten, ontbrekende leestoegang en
mislukte reads. Een ontgrendelwrite is geen bewijs van een juiste code.
De klokretry blijft begrensd. Codes en wachtwoorden worden niet gelogd.
Beta.4 voegt de eerste disconnectreden, uitleesstap en timing toe zonder het
protocol of de wachttijden te wijzigen. Het eigen opruimen van een client
wordt niet als sensorfout geteld.

## Softwarecontrole

De ESP32-build, `npm run build` en `npm test` zijn geslaagd. De tests omvatten
de React-app, de productie-C++-kern en de echte releasebestanden. De sessietests
controleren behoud van foutdetails, herstel na een tijdelijke klokfout en een
ontbrekende optionele temperatuur-read. Nieuwe diagnosetests controleren behoud
van de eerste reden en fase na retry, timing over klokoverloop en het negeren
van opruimevents. De eerdere GitHub-controle van beta.3 is ook geslaagd.

## Bewijs uit de fysieke test

De gedeelde diagnostics melden beta.3, 4 MB flash, verbonden wifi en MQTT,
en gesynchroniseerde tijd. Dit bevestigt nog geen ontvangst van sensormetingen
in Home Assistant.

| Sensorplek | Laatste waarneming | Wat dit aantoont |
|---|---|---|
| 1 | Verbinden mislukt, status 13 of 574 | Geen Bluetooth-verbinding; code niet verstuurd |
| 2 | Verbinden lukt; negen GATT-punten; klok heeft `read=1`; ontgrendelwrite lukt; verbinding verbroken vóór klok-read | Discovery en versturen werken; code en uitlezing nog niet bevestigd |
| 3 | Verbinden mislukt, status 574 | Ook hier nog geen geslaagde uitlezing |

Alle drie de plekken hadden aanvankelijk dezelfde 16-tekencode. De gebruiker
heeft bevestigd dat nog niet vaststaat of die code bij plek 2 hoort. Er is
bij geen sensor een geslaagde meting ontvangen. Status 574 is een mislukte
verbindingsopbouw en ontstaat vóór het versturen van een code. De reden van
het verbreken bij plek 2 staat nog niet in de beta.3-log.

Op 12 oktober bevestigt de gebruiker dat telefoon-Bluetooth uit stond en
sensor 2 wel met de CTEK-app werkt. Na een powercycle en **Nu uitlezen** is
hetzelfde foutverloop gemeld. Na het verzoek de eigen code opnieuw op te slaan
begint de pogingenteller van plek 2 opnieuw; de afgeronde log bevat opnieuw
dezelfde uitval. Dit is geen expliciete bevestiging dat de code van het eigen
label is gecontroleerd. **Bezig met BLE-uitlezing** verwijst naar de lopende
poging; de zichtbare log hoort dan nog bij de vorige afgeronde poging.

Deze notitie bevat geen echte sensoradressen, codes, wachtwoorden of ruwe
persoonlijke logs. De eigen instellingen blijven op de ESP32 opgeslagen.

## Eerstvolgende test: disconnectreden met beta.4 vastleggen

1. Download [de OTA-bin van beta.4](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/raw/refs/heads/main/firmware/releases/0.2.0-beta.4/ctek-esp32-wroom-32-ota.bin).
2. Kies **Uitlezen pauzeren en firmware bijwerken**, wacht de actieve poging
   af en upload de OTA-bin. Controleer na herstart **0.2.0-beta.4**.
3. Houd telefoon-Bluetooth uit en de ESP32 dichtbij de sensor in **plek 2**.
4. Klik op **Nu uitlezen**. Wacht tot **Bezig met BLE-uitlezing** verdwenen is
   en vernieuw de pagina.
5. Download diagnostics. De nieuwe regel `BLE-disconnect` noemt de reden,
   fase en tijd sinds het begin van de poging en ontgrendelwrite. Bewaar de
   log privé en verwijder adressen vóór openbaar delen.

Bij **Meting ontvangen** is een uitlezing met de huidige code bevestigd.
Controleer dan nog een uitlezing om te testen of herhaald uitlezen werkt.

Als de verbinding opnieuw vóór de klok-read verbreekt, beoordeel de nieuwe
reden en fase voordat het protocol wordt veranderd. Een disconnectreden
bewijst op zichzelf geen verkeerde code. De eigen code moet nog tegen het
sensorlabel of die exact gebruikte code in de officiële app bevestigd worden;
deel de code zelf niet. Er is nog geen fysieke sensortest van beta.4.

## Codespace stoppen en hervatten

De ESP32 draait zelfstandig en gebruikt de Codespace niet voor het uitlezen
of MQTT. De gepubliceerde firmwaredownload blijft op GitHub beschikbaar.
Na opslaan en pushen kan de Codespace worden gestopt. Om weer aan de
React-workshopapp te werken, start je de Codespace en gebruik je zo nodig
`npm start` op poort 3000. De beheerpagina van de ESP32 open je via het
eigen netwerkadres van het bord.
