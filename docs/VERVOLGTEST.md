# Teststatus en verdergaan

Bijgewerkt op 11 oktober 2026. De gebruiker hervat de fysieke tests op
12 oktober 2026. Eigenaar en beheerder: bbartje.

## Opgeslagen firmware

- Actuele versie: **0.2.0-beta.3**, voor ESP32-WROOM-32 met 4 MB flash.
- De firmware en flashbestanden zijn gecommit en gepusht naar GitHub in
  commit `8db6863f0194155364d02ce2601a1a3342cab8cb`.
- [De publieke bètarelease](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/tag/v0.2.0-beta.3)
  is met expliciete toestemming van de gebruiker gepubliceerd.
- [De OTA-bin voor een wifi-update](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.3/ctek-esp32-wroom-32-ota.bin?download=1)
  is 1.181.392 bytes. Grootte en SHA-256 van de publieke download zijn
  gecontroleerd tegen de lokale binary.
- Lokale bestanden staan in `firmware/releases/0.2.0-beta.3/`; de ZIP staat
  één map hoger. Beta.1 en beta.2 blijven beschikbaar.

Beta.3 zoekt alle GATT-punten daadwerkelijk op, logt hun mogelijkheden en
maakt onderscheid tussen ontbrekende punten, ontbrekende leestoegang en
mislukte reads. Een ontgrendelwrite is geen bewijs van een juiste code.
De klokretry blijft begrensd. Codes en wachtwoorden worden niet gelogd.

## Softwarecontrole

De ESP32-build, `npm run build` en `npm test` zijn geslaagd. De tests omvatten
de React-app, de productie-C++-kern en de echte releasebestanden. De nieuwe
sessietests controleren behoud van foutdetails, herstel na een tijdelijke
klokfout en een ontbrekende optionele temperatuur-read. De GitHub-controle
van de firmwarecommit is ook geslaagd.

## Bewijs uit de fysieke test

De gedeelde diagnostics melden beta.3, 4 MB flash, verbonden wifi en MQTT,
en gesynchroniseerde tijd. Dit bevestigt nog geen ontvangst van sensormetingen
in Home Assistant.

| Sensorplek | Laatste waarneming | Wat dit aantoont |
|---|---|---|
| 1 | Verbinden mislukt, status 574 | Geen Bluetooth-verbinding; code niet verstuurd |
| 2 | Verbinden lukt; negen GATT-punten; klok heeft `read=1`; ontgrendelwrite lukt; verbinding verbroken vóór klok-read | Discovery en versturen werken; code en uitlezing nog niet bevestigd |
| 3 | Vorige poging: verbinden mislukt, status 574; volgende poging liep tijdens downloaden | Ook hier nog geen geslaagde uitlezing |

Alle drie de plekken hadden aanvankelijk dezelfde 16-tekencode. De gebruiker
heeft bevestigd dat nog niet vaststaat of die code bij plek 2 hoort. Er is
bij geen sensor een geslaagde meting ontvangen. Status 574 is een mislukte
verbindingsopbouw en ontstaat vóór het versturen van een code. De reden van
het verbreken bij plek 2 staat nog niet in de log.

Deze notitie bevat geen echte sensoradressen, codes, wachtwoorden of ruwe
persoonlijke logs. De eigen instellingen blijven op de ESP32 opgeslagen.

## Eerstvolgende test: één sensor herstarten

De gebruiker merkt op dat een herstart van de Battery Sense ook bij verbinden
met de telefoon nodig blijkt. CTEK beschrijft minimaal 30 seconden zonder
sensorvoeding en daarna opnieuw aansluiten voor de herstelroute bij een
vergeten code. Dat maakt dit een relevante test; het bewijst niet dat elke
normale uitlezing een herstart vereist.
[CTEK-productinformatie en veelgestelde vragen](https://www.ctek.it/dettaglio-modello.php?id=62).

1. Begin met de sensor in **plek 2** en zet de ESP32 dichtbij.
2. Zet Bluetooth op de betrokken telefoons uit.
3. Onderbreek de voeding van alleen deze Battery Sense-sensor minimaal
   **30 seconden** en sluit hem weer aan volgens zijn aansluitinstructies.
4. Laat de huidige code staan, zodat alleen het effect van de herstart wordt
   getest. Herstart of verander de andere sensoren nog niet.
5. Klik op de ESP32-beheerpagina op **Nu uitlezen**.
6. Download daarna de nieuwe diagnostics. Bewaar die privé en verwijder
   sensoradressen voordat je ze openbaar deelt.

Bij **Meting ontvangen** is een uitlezing met de huidige code bevestigd.
Controleer dan nog een uitlezing zonder opnieuw de sensorvoeding te
onderbreken. Zo testen we ook of herhaald uitlezen zelfstandig werkt.

Als de verbinding opnieuw vóór de klok-read verbreekt, controleer eerst de
bijbehorende code tegen het sensorlabel of een geslaagde uitlezing met die
exacte code in de officiële CTEK-app. Zet de telefoon-Bluetooth daarna weer
uit. Bij een bevestigde code en aanhoudende fout kan een volgende gerichte
firmwarewijziging de reden en het moment van het verbreken vastleggen.
Er is nog geen volgende firmwareversie gemaakt.

## Codespace stoppen en hervatten

De ESP32 draait zelfstandig en gebruikt de Codespace niet voor het uitlezen
of MQTT. De gepubliceerde firmwaredownload blijft op GitHub beschikbaar.
Na opslaan en pushen kan de Codespace worden gestopt. Om weer aan de
React-workshopapp te werken, start je de Codespace en gebruik je zo nodig
`npm start` op poort 3000. De beheerpagina van de ESP32 open je via het
eigen netwerkadres van het bord.
