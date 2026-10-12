# Teststatus en verdergaan

Bijgewerkt op 12 oktober 2026. Eigenaar en beheerder: bbartje.

## Huidige stand voor eigenaar en meelezers

De eigenaar gaat beta.6 via OTA (een firmware-update over wifi) installeren en
testen en meldt de resultaten daarna. De installatie en de testuitslag zijn
nog niet bevestigd. Deze notitie bewaart de stand om later verder te gaan.

| Onderdeel | Vastgelegde stand |
|---|---|
| Code en flashpakket beta.6 | Gecommit en op GitHub in commit `4a91612`. |
| OTA-download | Beschikbaar; op 12 oktober opnieuw gedownload met HTTP 200, 1.208.528 bytes. |
| Laatst bevestigde versie op de ESP32 | Beta.4; beta.6-installatie nog niet bevestigd. |
| Eerder hardwarebewijs | Eén volledige uitlezing van sensorplek 3 met beta.4 en ontvangst in Home Assistant. |
| Volgende stap | Eigenaar installeert beta.6 en voert de acceptatietest uit; resultaten volgen. |

Begin bij de [OTA-download voor beta.6](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/raw/refs/heads/main/firmware/releases/0.2.0-beta.6/ctek-esp32-wroom-32-ota.bin)
en de [wifi-updatehandleiding](FLASHEN.md#5-update-firmware-over-wi-fi).
Lees voor de wijzigingen de [releasenotities](RELEASE_0.2.0-beta.6.md),
voor de bekende aandachtspunten het
[kwaliteitsrapport](../KWALITEITSRAPPORT-2026-10-12.md) en voor de controles de
[acceptatietest](ACCEPTATIETEST.md). Lege testvakken betekenen **niet getest**.
De softwarecontroles zijn geslaagd; de fysieke beta.6-test en externe
acceptatie staan nog open. Er is nog geen stabiele versie 1.0.0 vrijgegeven.

## Opgeslagen firmware

Nieuwste versie: **0.2.0-beta.6**, voor ESP32-WROOM-32 met 4 MB flash.
Bron en flashbestanden staan in deze repository; de eerdere pakketten blijven.

- De beta.6-code en het gecontroleerde flashpakket staan op GitHub in commit
  `4a91612f52d3eb6de8f53ce3ff553b039ffb6b04`.
- De nieuwe bestanden staan in `firmware/releases/0.2.0-beta.6/`; de ZIP
  staat één map hoger. De OTA-download is direct beschikbaar vanuit de
  repository. Zie [de releasenotities](RELEASE_0.2.0-beta.6.md).

### Eerdere vrijgave: beta.3

- De eerdere beta.3-firmware en flashbestanden zijn gecommit en gepusht in
  commit `8db6863f0194155364d02ce2601a1a3342cab8cb`.
- [De publieke bètarelease](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/tag/v0.2.0-beta.3)
  is met expliciete toestemming van de gebruiker gepubliceerd.
- [De OTA-bin voor een wifi-update](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/releases/download/v0.2.0-beta.3/ctek-esp32-wroom-32-ota.bin?download=1)
  is 1.181.392 bytes. Grootte en SHA-256 van de publieke download zijn
  gecontroleerd tegen de lokale binary.

### Wijzigingen sinds beta.3

Beta.6 voegt herstel-wifi voor twee minuten na opstart, Nederlands/Engels/Duits,
een tijdzonekeuze, progressieve sensorplekken en live Home Assistant-waarden toe.
De softwarelogica is niet hetzelfde als hardwarebewijs: de live BLE-punten,
herhaald uitlezen, MQTT-ontvangst, herstel-login en OTA moeten op de ESP32 nog
worden bevestigd.

De kwaliteitscontrole heeft het herstelwachtwoord beperkt tot wachtwoordherstel
via het setupnetwerk; instellingen, diagnostics en OTA vereisen het eigen
wachtwoord. Ook zijn de tijdzone-overwrite, het verkeerd benoemen van
historische spanning en de multipart-parserkwetsbaarheid gecorrigeerd.
Gebruik [de acceptatietest](ACCEPTATIETEST.md) voor de laatste eigen test.
Beta.6 blijft een testkandidaat; de volgende fase is een beperkte externe bèta,
en pas daarna kan een stabiele 1.0.0 volgen. Er is nog geen nieuwe fysieke
uitslag vastgelegd.

Beta.3 zoekt alle GATT-punten daadwerkelijk op, logt hun mogelijkheden en
maakt onderscheid tussen ontbrekende punten, ontbrekende leestoegang en
mislukte reads. Een ontgrendelwrite is geen bewijs van een juiste code.
De klokretry blijft begrensd. Codes en wachtwoorden worden niet gelogd.
Beta.4 voegt de eerste disconnectreden, uitleesstap en timing toe zonder het
protocol of de wachttijden te wijzigen. Het eigen opruimen van een client
wordt niet als sensorfout geteld.
Beta.5 stelt in MQTT Discovery twee decimalen voor spanning en één voor beide
temperaturen voor. De oorspronkelijke numerieke meetwaarden blijven behouden.

## Softwarecontrole

De ESP32-build, `npm run build` en `npm test` zijn geslaagd. De tests omvatten
de React-app, de productie-C++-kern en de echte releasebestanden. De sessietests
controleren behoud van foutdetails, herstel na een tijdelijke klokfout en een
ontbrekende optionele temperatuur-read. Nieuwe diagnosetests controleren behoud
van de eerste reden en fase na retry, timing over klokoverloop en het negeren
van opruimevents. De eerdere GitHub-controle van beta.3 is ook geslaagd.

## Bewijs uit de fysieke test

De eerdere diagnostics melden beta.3, 4 MB flash, verbonden wifi en MQTT,
en gesynchroniseerde tijd. De nieuwste test met beta.4 bevestigt voor plek 3
een volledige sensoruitlezing en metingen op de beheerpagina. Ontvangst van
spanning en temperatuur in Home Assistant is daarna door de gebruiker gemeld.

| Sensorplek | Laatste waarneming | Wat dit aantoont |
|---|---|---|
| 1 | Verbinden mislukt, status 13 of 574 | Geen Bluetooth-verbinding; code niet verstuurd |
| 2 | Verbinden lukt; negen GATT-punten; klok heeft `read=1`; ontgrendelwrite lukt; verbinding verbroken vóór klok-read | Discovery en versturen werken; code en uitlezing nog niet bevestigd |
| 3 | Beta.4: verbinden lukt; negen GATT-punten; metadata en temperatuur gelezen; twee historierecords volledig ontvangen; `Meting ontvangen` | Eén volledige fysieke uitlezing; huidige code en protocol voor deze sensor bevestigd |

Alle drie de plekken hadden aanvankelijk dezelfde 16-tekencode. De gebruiker
had toen bevestigd dat nog niet vaststond of die code bij plek 2 hoorde.
Tijdens die eerdere tests was nog bij geen sensor een geslaagde meting ontvangen.
Status 574 is een mislukte verbindingsopbouw en ontstaat vóór het versturen van
een code. De reden van het verbreken bij plek 2 staat nog niet in de beta.3-log.

Op 12 oktober bevestigt de gebruiker dat telefoon-Bluetooth uit stond en
sensor 2 wel met de CTEK-app werkt. Na een powercycle en **Nu uitlezen** is
hetzelfde foutverloop gemeld. Na het verzoek de eigen code opnieuw op te slaan
begint de pogingenteller van plek 2 opnieuw; de afgeronde log bevat opnieuw
dezelfde uitval. Dit is geen expliciete bevestiging dat de code van het eigen
label is gecontroleerd. **Bezig met BLE-uitlezing** verwijst naar de lopende
poging; de zichtbare log hoort dan nog bij de vorige afgeronde poging.

### Eerste geslaagde uitlezing van plek 3 met beta.4

De gebruiker heeft de sensor in plek 3 door uitschakelen geïdentificeerd en
de bijbehorende code opgeslagen. Aanvankelijk mislukte verbinden nog met
status 574; de code werd toen niet verstuurd. Een scan zag dezelfde sensor
met adrestype 0, als verbindbaar, en met een signaal van −90 dBm. De gebruiker
heeft de afstand gecorrigeerd naar ongeveer vier meter, zonder metaal ertussen.

Na het verzoek op ongeveer één meter opnieuw te scannen en uit te lezen,
bevat de volgende gedeelde beta.4-log:

- `connect=1 status=0 (SUCCESS)` en negen gevonden GATT-punten.
- Een geslaagde ontgrendelwrite, gevolgd door voltooide reads van klok,
  meetperiode, historiecursor en actuele temperatuur.
- Geslaagde notificatie-inschakeling en ontvangst van alle twee aangevraagde
  historierecords, zonder ongeldige records.
- `Meting ontvangen`; de beheerpagina toont actuele spanning en historische
  en actuele temperatuur. Er is vóór het eigen opruimen geen disconnect gezien.

Dit bevestigt de huidige code en een volledige uitlezing van deze sensor.
De pagina meldt twee pogingen, één geslaagd en nul opeenvolgende fouten;
de tweede poging staat nog op **Bezig met BLE-uitlezing**. Herhaald succes
is dus nog niet bevestigd. De afstand en signaalsterkte bij de succesvolle
test zijn nog niet expliciet gemeld. Bereik is een mogelijke verklaring
voor de eerdere verbindingsfout, maar de oorzaak is hiermee niet bewezen.

### Home Assistant en gewenste decimalen

De gebruiker meldt daarna dat de spanning in Home Assistant op hele volts
werd weergegeven en handmatig kon worden ingesteld op meer decimalen.
Temperatuur verscheen al met één decimaal. Daarmee is ontvangst van spanning
en temperatuur in Home Assistant gemeld; herhaald bijwerken is nog niet
afzonderlijk bevestigd. Beta.5 maakt twee decimalen voor spanning en één voor
temperatuur de voorgestelde standaard. Een eigen instelling in Home Assistant
blijft voorgaan. De nieuwe Discovery-weergave moet nog fysiek worden getest.

Deze notitie bevat geen echte sensoradressen, codes, wachtwoorden of ruwe
persoonlijke logs. De eigen instellingen blijven op de ESP32 opgeslagen.

## Historische beta.5-status

De gebruiker wil vanavond verder testen met een andere sensor. Beta.5 is
gecommit en naar GitHub gepusht in commit `5807d2a`; de publieke OTA-download
is byte voor byte vergeleken met het geteste lokale bestand en is gelijk.
De nieuwste expliciet gemelde firmware op de ESP32 is beta.4. Installatie
van beta.5 en de nieuwe standaardweergave zijn nog niet bevestigd.

Gebruik voor de volgende sensor het bijbehorende adres, het adrestype uit de
scan en de eigen code. Begin met de ESP32 dichtbij en telefoon-Bluetooth uit.
Beoordeel één afgeronde poging tegelijk. Noteer de sensorplek, firmwareversie,
scan-signaalsterkte en het resultaat; deel geen codes of wachtwoorden.
De werkende configuratie van plek 3 blijft het referentiepunt.

## Eerstvolgende test: beta.6 op hardware controleren

1. Installeer beta.6 via de [wifi-updatehandleiding](FLASHEN.md#5-update-firmware-over-wi-fi).
   Pauzeer het uitlezen, wacht tot de lopende poging klaar is en upload de
   OTA-bin. Controleer na herstart versie **0.2.0-beta.6** en het behoud van
   wifi-, MQTT- en sensorinstellingen en je eigen wachtwoord.
2. Begin met de eerder werkende sensor van plek 3. Houd telefoon-Bluetooth uit
   en wacht op een afgeronde poging met **Meting ontvangen**. Controleer live
   spanning en actuele temperatuur op de gateway en in Home Assistant.
3. Controleer de lage-spanningsstatus, `BartjeB` bij apparaatinfo en het opruimen
   van de oude dubbele temperatuur- en RSSI-entiteiten. Eigen Home
   Assistant-weergaveprecisie blijft leidend. Controleer ook de lokale tijd.
4. Laat meerdere automatische uitleesrondes lopen. Controleer dat de geslaagde
   teller oploopt en Home Assistant verse updates ontvangt. Noteer afstand en
   scan-signaalsterkte voor de werkende opstelling.
5. Test de overige sensoren één voor één met hun eigen code en adres. Werk de
   resterende [acceptatiecontroles](ACCEPTATIETEST.md) af, inclusief herstel,
   verbindingsuitval en de duurtest. Noteer wat je nog niet hebt getest.

Bij terugkomst zijn firmwareversie, OTA-uitkomst, aantal sensoren, testduur en
geslaagde of mislukte controles voldoende om verder te gaan. Deel geen codes,
wachtwoorden of echte sensoradressen. Er worden pas controles als geslaagd
vastgelegd nadat de eigenaar de uitslag heeft gemeld.

## Bij een volgende fout

Wacht tot **Bezig met BLE-uitlezing** verdwenen is en vernieuw de pagina;
deel daarna de afgeronde BLE-log of diagnostics zonder sensoradressen.
Bij `connect=0` is de code nog niet verstuurd: controleer scan, adrestype
en ontvangst. Als verbinden lukt en de verbinding na de ontgrendelwrite
verbreekt, beoordeel de beta.4-regel `BLE-disconnect` met reden, fase en timing.
Een disconnectreden bewijst op zichzelf geen verkeerde code. Deel de code
zelf niet. De succesvolle test van plek 3 bewijst nog niet de oorzaak van
de eerdere uitval bij plek 2 of de juiste code voor een andere sensor.

## Codespace stoppen en hervatten

De ESP32 draait zelfstandig en gebruikt de Codespace niet voor het uitlezen
of MQTT. De gepubliceerde firmwaredownload blijft op GitHub beschikbaar.
Na opslaan en pushen kan de Codespace worden gestopt. Om weer aan de
React-workshopapp te werken, start je de Codespace en gebruik je zo nodig
`npm start` op poort 3000. De beheerpagina van de ESP32 open je via het
eigen netwerkadres van het bord.
