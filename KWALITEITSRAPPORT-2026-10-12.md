# Kwaliteitsrapport: CTEK BLE Gateway ESP32

**Datum:** 12 oktober 2026 · **Beoordeeld door:** Codex (GPT-6) ·
**Beoogd niveau:** 3, praktische tool voor een beperkte groep ·
**Gebruikers:** eigenaar/beheerder bbartje en externe testers met eigen ESP32,
CTEK-sensoren, wifi en Home Assistant · **Gegevens:** echte batterijmetingen,
sensoradressen, sensornamen, tijden en lokaal opgeslagen verbindingsgegevens.

## Samenvatting

De gateway leest maximaal vier CTEK Battery Sense-sensoren om de beurt en stuurt
metingen naar de eigen MQTT-broker en Home Assistant. De beta.6-code en het
bijbehorende flashpakket zijn gecontroleerd, verbeterd en naar GitHub gepusht
in commit `4a91612`. De lokale builds en tests slagen, inclusief controles van
de werkelijk geleverde binaries. De nieuwe versie is klaar voor de laatste
eigen praktijktest; zonder die uitslag is dagelijks gebruik door anderen nog
niet voldoende onderbouwd. Houd de bètastatus tijdens externe tests en maak
1.0.0 pas na geslaagde tests op hun eigen hardware en netwerken.

**Stand bij overdracht op 12 oktober 2026:** de eigenaar heeft aangekondigd
beta.6 via OTA te installeren en te testen. De OTA-download is beschikbaar;
installatie en fysieke testresultaten zijn nog niet bevestigd. Volg de
[acceptatietest](docs/ACCEPTATIETEST.md); de
[teststatus en vervolgstappen](docs/VERVOLGTEST.md) bewaren de actuele stand.

**Past de app bij het beoogde niveau?** Bijna.
**Huidig niveau volgens deze toets:** functionele bèta met beheermaatregelen
voor niveau 3; volledige praktijkacceptatie ontbreekt nog.

## Scorekaart

🟢 in orde · 🟠 aandachtspunt · 🔴 nodig vóór normaal dagelijks gebruik ·
⚪ niet te controleren of niet van toepassing.

| Onderdeel | Score | Toelichting in één zin |
|---|---|---|
| 1. Werkt het? | 🟠 | Software bouwt; de complete beta.6-route op echte hardware is nog niet bevestigd. |
| 2. Veiligheid | 🟠 | Concrete fouten hersteld; openbaar herstelvenster en onversleutelde lokale verbindingen blijven bewuste beperkingen. |
| 3. Gegevens en privacy | 🟠 | Metingen blijven bij de tester, maar gedeelde diagnostiek moet handmatig worden geanonimiseerd. |
| 4. Versiebeheer | 🟢 | Gecontroleerde code en flashpakket gecommit en gepusht; eerdere releases blijven beschikbaar. |
| 5. Structuur en onderhoudbaarheid | 🟢 | Protocol, Bluetooth, netwerk, MQTT en beheer zijn gescheiden en zonder nieuwe runtimebibliotheek verbeterd. |
| 6. Testen en foutafhandeling | 🟠 | Goede geautomatiseerde kerncontroles; hardware- en opslagfoutscenario's zijn nog niet volledig afgedekt. |
| 7. Documentatie | 🟢 | Installatie, OTA, privacy, beperkingen en concrete acceptatielijst zijn beschikbaar, ook in de ZIP. |
| 8. Beheer en borging | 🟠 | Eigenaar benoemd; iedere tester beheert eigen instellingen en Home Assistant-back-ups, herstel moet praktisch worden bewezen. |

## Wat gaat goed

- De productiecode voor protocol en sessies wordt op de computer getest, met
  controles die geheugenfouten kunnen vinden. Geen succesclaim op basis van
  alleen een verzonden ontgrendelbericht (`firmware/test/session_test.cpp:76`).
- De uitgeleverde factory- en OTA-images worden gecontroleerd op chip, versie,
  checksums, flashindeling, beschadigde bestanden en overeenkomst met de
  bronnen (`firmware/tools/test_release.py:30`).
- Eigen wachtwoord verplicht vóór normaal gebruik; formuliercontrole beschermt
  wijzigingen en updates. Geheimen komen niet in ingevulde wachtwoordvelden
  of diagnostiek terug (`firmware/src/web_gateway.cpp:84`, `:175`, `:330`).
- Een eerdere echte uitlezing met beta.4 en ontvangst in Home Assistant staan
  als door de gebruiker gemeld bewijs beschreven, afzonderlijk van
  softwaretests ([vastgelegd testbewijs](docs/VERVOLGTEST.md#bewijs-uit-de-fysieke-test)).

## Verkenning en uitgevoerde controles

Het product is C++-firmware voor ESP32-WROOM-32 met Arduino, NimBLE, PubSubClient
en ArduinoJson. De React 18/Vite-app is alleen de workshopstartpagina en draait
los van de ESP32-beheerpagina (zie de [README](README.md#run-the-workshop-app)
en `src/App.jsx:3`). Python verzorgt
builds en releasecontroles. De workshopinstructies en omgeving zijn niet als
productfunctionaliteit beoordeeld.

Instellingen staan in NVS: het vaste configuratiegeheugen van de ESP32.
Meetresultaten staan tijdelijk in RAM; Home Assistant bewaart eigen historie.
Metingen gaan naar de eigen broker. Tijd wordt opgehaald bij `pool.ntp.org` en
`time.cloudflare.com`; daarbij worden geen batterijmetingen meegestuurd
(`firmware/src/config_network.cpp:149`, `firmware/src/mqtt_gateway.cpp:109`).
De firmware gebruikt geen AI-dienst of CTEK-cloudaccount.

| Echt uitgevoerd | Uitkomst |
|---|---|
| `npm run build` | Geslaagd. |
| `npm test` | Geslaagd: 7 JavaScript-tests, 7 C++-testsuites, aparte uploadbeveiligingstest en releasecontroles. |
| PlatformIO Core 6.1.19, ESP32-build | Geslaagd; statisch RAM 20,6%, app-flash 61,1%. Dit bewijst geen vrij runtimegeheugen tijdens een duurtest. |
| `python3 firmware/tools/build_release.py --pio /tmp/platformio-venv/bin/pio` | Geslaagd; factory-bin, OTA-bin, manifest, checksums en ZIP gebouwd. |
| `npm audit --omit=dev` | 0 bekende kwetsbaarheden in npm-productieafhankelijkheden; dit controleert geen ESP32-bibliotheken. |
| `git diff --check` | Geslaagd. |
| `git status`, `git log --oneline -n 30`, `git remote -v`, push | Bestaande Copilot-wijzigingen bij aanvang aanwezig; gecontroleerde stap gepusht naar `origin/main`. |
| Zoektocht naar geheimen, huidige tekstbestanden en Git-geschiedenis | Geen herkenbare privé-API-sleutels/tokens of hardcoded eigen wachtwoorden gevonden met gerichte patronen. |

Er is geen lintscript. `node_modules` bestond al; afhankelijkheden zijn niet
opnieuw geïnstalleerd. Zoekcontroles vonden geen `eval`, `innerHTML` of
`dangerouslySetInnerHTML` in de productiecode. HTML-invoer wordt ontsmet door
speciale tekens te vervangen (`firmware/src/web_gateway.cpp:62`). Bekende
geheimenpatronen zijn ook met `git log --all -G ... --name-only` gecontroleerd,
zonder eventuele waarden te publiceren. `.env`, `node_modules`, `dist` en
`.pio` worden genegeerd. De regel voor `.env.*` is toegevoegd zodat ook andere
lokale omgevingsbestanden worden uitgesloten; alleen een fictief
`.env.example` mag expliciet worden toegevoegd (`.gitignore:72`).

De eerste `npm test` mislukte terecht omdat het beta.6-manifest nog ontbrak.
Na het bouwen van het pakket slaagt dezelfde controle. De sandbox kon hier
geen processen starten; bouwen/testen/Git zijn na tooltoestemming buiten die
sandbox uitgevoerd. Dat was een omgevingsbeperking, geen appfout.

## Bevindingen

### Hersteld tijdens deze controle

1. **🔴 → opgelost: kwetsbare uploadparser.** De gebruikte Arduino-parser
   reserveerde tijdelijk geheugen op basis van een onbeperkt scheidingsteken
   uit de upload. De bouwstap compileert nu een kopie met de limiet van 70 tekens
   vóór die reservering, zonder de SDK-cache te wijzigen. De grensgevallen
   worden uitgevoerd in een C++-test (`firmware/tools/webserver_security.py:10`,
   `firmware/test/webserver_security_test.py:29`). Dit is de gerichte correctie
   voor [CVE-2026-42854 bij Espressif](https://github.com/espressif/arduino-esp32/security/advisories/GHSA-8cmm-3887-r32j).
2. **🔴 → opgelost: openbaar herstelwachtwoord gaf volledige toegang.** De
   openbare login is beperkt tot wachtwoordherstel op het setupadres en vanuit
   het setup-subnet; configuratie, diagnostiek en OTA vereisen het eigen
   wachtwoord. Alleen HTTP Basic-aanmeldingen worden geaccepteerd
   (`firmware/src/web_gateway.cpp:36`, `firmware/include/admin_access.h:42`).
3. **🟠 → opgelost: tijdzone werd teruggezet naar UTC.** `configTzTime` bewaart
   nu de gekozen zomer-/wintertijdregels. De test controleert daadwerkelijke
   tijdomrekening, ook rond de Nederlandse zomertijdwisseling
   (`firmware/src/config_network.cpp:149`, `firmware/test/time_zone_test.cpp:18`).
4. **🟠 → opgelost: historie kon als actuele spanning of waarschuwing worden
   gepresenteerd.** Historische fallback is benoemd; ontbrekende live-spanning
   en lage-spanningsstatus worden null. De Home Assistant-template zet dat om
   naar onbekend (`firmware/src/web_gateway.cpp:222`,
   `firmware/src/mqtt_gateway.cpp:67`, `firmware/include/telemetry.h:21`).
   Dit sluit aan bij de [officiële MQTT binary-sensordocumentatie](https://www.home-assistant.io/integrations/binary_sensor.mqtt/).
5. **🟠 → opgelost: onvoltooide testkandidaat en fragiele browserbediening.**
   Het ontbrekende pakket is gebouwd. Taalkeuze en plusknop blijven werken als
   browseropslag geblokkeerd is; de plus werkt ook vóór de eerste opgeslagen
   sensor. Eigen beheerwachtwoorden worden niet meer naar serieel geschreven
   (`firmware/include/web_ui.h:19`, `src/firmware-ui.test.js:42`,
   `firmware/src/config_network.cpp:122`).

### Nog open

1. **🔴 Volledige fysieke acceptatie ontbreekt vóór dagelijks gebruik.**
   Beta.6 voegt een live-spanningsread en herstelgedrag toe die niet op deze
   werkplek met echte hardware zijn getest. Ook alle sensoren samen, langdurig
   draaien, MQTT-herstel en OTA vereisen nog bewijs. De eerdere beta.4-uitlezing
   bewijst dit niet (zie de [vastgelegde teststatus](docs/VERVOLGTEST.md)). Gebruik
   `docs/ACCEPTATIETEST.md:14`; pas daarna een beperkte externe bèta starten.
2. **🟠 Herstel blijft een openbaar venster binnen radiobereik.** Iemand vlakbij
   kan tijdens twee minuten na opstart het wachtwoord vervangen en daarna
   beheer krijgen. De eigenaar heeft deze keuze expliciet geaccepteerd.
   De beperking tot het setupnetwerk moet nog fysiek worden getest
   (`firmware/src/config_network.cpp:25`, `firmware/src/web_gateway.cpp:40`).
   HTTP, MQTT en NVS zijn bovendien onversleuteld; dit past alleen bij het
   afgesproken vertrouwde thuisnetwerk (zie de [README](README.md#data-and-privacy)).
3. **🟠 Opslagfouten bij gewone instellingen/sensoren worden niet gemeld.**
   `saveConfiguration` en `saveSensors` controleren de uitkomst van de
   NVS-schrijfacties niet. De gebruiker kan daardoor bij een echte opslagfout
   een succesreactie krijgen terwijl niet alles bewaard is. De wachtwoordroute
   controleert dit wel (`firmware/src/config_network.cpp:40`, `:86`, `:98`;
   `firmware/src/web_gateway.cpp:283`, `:310`). Geen opslagfout waargenomen;
   dit is een vastgesteld gat in de foutafhandeling, geen bewezen dataverlies.
4. **🟠 Offline verwijderen laat Home Assistant-configuratie achter.**
   `removeDiscovery` doet niets zonder brokerverbinding. Verwijder daarom
   tijdens de bèta alleen wanneer MQTT verbonden is, of ruim oude
   configuratieberichten handmatig op (`firmware/src/mqtt_gateway.cpp:191`,
   `firmware/README.md:178`). Automatisch opruimen na herstel is een verbetering
   voor een volgende versie.
5. **🟠 Diagnostiek is niet automatisch anoniem.** JSON bevat sensoradressen,
   BLE-logs en meetwaarden. Deel uitsluitend gecontroleerde, geanonimiseerde
   kopieën (`firmware/src/web_gateway.cpp:342`, `docs/ACCEPTATIETEST.md:49`).
   Er is geen automatische export/back-up van gatewayinstellingen; bewaar
   codes en netwerkgegevens privé en gebruik Home Assistant-back-ups
   (zie de [README](README.md#data-and-privacy)).
6. **🟠 Vertalingen en bibliotheekonderhoud hebben nog grenzen.** Een aantal
   foutreacties blijft Nederlands, ook als Engels/Duits gekozen is
   (`firmware/src/web_gateway.cpp:270`). De oude Arduino-core heeft nu een
   gerichte parsercorrectie; een complete audit van alle SDK-componenten is
   niet uitgevoerd. Bij een andere core-versie moet de beveiligingspatch
   opnieuw worden beoordeeld (`firmware/tools/webserver_security.py:14`).
7. **⚪ Rechten op meegeleverde analyse-input niet vastgesteld.** Het XAPK
   staat in Git (`git ls-files CTEK+Battery+Sense_2.4.6_APKPure.xapk`), maar niet
   in het flashpakket. Librarylicenties en notices zijn meegeleverd. Of de
   oorspronkelijke Android-app mag worden herverdeeld is niet onderzocht;
   dit rapport doet daarover geen juridische uitspraak
   (`firmware/THIRD_PARTY_NOTICES.md:23`).

## Top 5 verbeteracties

Voer deze acties één voor één uit. Test en commit na iedere softwarewijziging.

### 1. Rond de eigen acceptatietest af

- **Waarom:** softwaretests bewijzen radio, voeding, MQTT en OTA niet.
- **Moeite:** middel; minimaal een dag doorlooptijd voor de duurtest.
- **Zelf te doen met AI?** Ja; de maker voert de hardwarehandelingen uit.
- **Opdracht voor je AI-assistent:**

  ```
  Begeleid mij door docs/ACCEPTATIETEST.md voor beta.6. Bespreek één test
  tegelijk, bewaar alleen geanonimiseerde uitslagen en markeer ontbrekend
  bewijs als niet getest. Beslis op basis van de resultaten of een beperkte
  externe bèta kan starten; claim geen stabiele 1.0 zonder praktijktest.
  ```

### 2. Verzamel zelfstandige externe testresultaten

- **Waarom:** ieder moet op eigen hardware/netwerk met alleen de handleiding
  kunnen installeren en herstellen.
- **Moeite:** middel.
- **Zelf te doen met AI?** Ja, samen met de testers.
- **Opdracht voor je AI-assistent:**

  ```
  Verwerk mijn geanonimiseerde externe beta.6-testresultaten in de bestaande
  testdocumentatie. Noteer hardware, Home Assistant-versie, aantal sensoren,
  installatie, herhaalde updates, verbindingsherstel, wachtwoordherstel en
  OTA. Maak onderscheid tussen geslaagd, mislukt en niet getest. Vraag
  nooit om wachtwoorden of sensorcodes. Onderbouw of 1.0.0 verantwoord is.
  ```

### 3. Maak bewaren van instellingen aantoonbaar betrouwbaar

- **Waarom:** een succesmelding moet betekenen dat gegevens echt bewaard zijn.
- **Moeite:** middel.
- **Zelf te doen met AI?** Ja, met extra beoordeling van opslag en migratie.
- **Opdracht voor je AI-assistent:**

  ```
  Verbeter de NVS-opslag van netwerk- en sensorinstellingen. Meld mislukte
  schrijfacties begrijpelijk, behoud de vorige geldige toestand en voorkom
  gedeeltelijke configuratie. Leg een eventuele opslagmigratie eerst ter
  beoordeling voor. Test opslagfouten en behoud na herstart; bouw een volgende
  bètaversie en flashpakket en controleer npm run build en npm test.
  ```

### 4. Ruim offline verwijderde sensoren na brokerherstel op

- **Waarom:** anders blijven ongebruikte entiteiten bij testers zichtbaar.
- **Moeite:** middel.
- **Zelf te doen met AI?** Ja.
- **Opdracht voor je AI-assistent:**

  ```
  Zorg dat Home Assistant Discovery van offline verwijderde/vervangen sensoren
  na MQTT-herstel wordt opgeruimd, ook na een gatewayherstart. Behoud nieuwe
  sensoren en andere gateways. Test verwijderen zonder broker, herstart en
  herstel; maak een volgende bètaversie met gecontroleerd flashpakket.
  ```

### 5. Verbeter ondersteuning en controleer distributiemateriaal

- **Waarom:** testers moeten fouten kunnen begrijpen en veilig melden.
- **Moeite:** klein voor teksten; middel voor rechten/licenties.
- **Zelf te doen met AI?** Ja voor teksten; juridische vragen met specialist.
- **Opdracht voor je AI-assistent:**

  ```
  Maak de overige gebruikersfouten begrijpelijk in Nederlands, Engels en
  Duits en voeg een optie voor geanonimiseerde diagnostiek toe. Controleer
  dat export geen sensoradressen, codes of wachtwoorden bevat. Inventariseer
  daarnaast welk materiaal van derden in repository en flashpakket zit,
  zonder zelf te beslissen dat herdistributie toegestaan is of geschiedenis
  te verwijderen. Test de wijzigingen en houd documentatie en pakket gelijk.
  ```

## Doorgroeien naar het volgende niveau

Voor niveau 4 zijn gedeeld technisch beheer, onafhankelijke beveiligingsreview,
monitoring, herstel/back-upbeleid en een bewust model voor netwerkbeveiliging
nodig. TLS, ondertekende firmware en automatische terugval na een mislukte
updateboot verdienen dan beoordeling. Dit vereist ontwerpkeuzes; ze zijn niet
als nieuwe bibliotheken of externe diensten aan deze beperkte bèta toegevoegd.

## Waar je een specialist of IT nodig hebt

Bij gebruik buiten het vertrouwde thuisnetwerk, commercieel uitrollen of hoge
beschikbaarheid: laat netwerkbeveiliging, SDK-onderhoud en updateherstel door
een ervaren embedded-ontwikkelaar beoordelen. Laat rechten en volledige
licentievoorwaarden rond appmateriaal en binaire distributie zo nodig door
een deskundige beoordelen. De batterijwaarde is een meting, geen gegarandeerde
diagnose of veiligheidssysteem.

## Beperkingen van deze toets

Geen fysiek bord/sensor of Home Assistant beschikbaar op deze werkplek.
Hardware-radioverkeer, MQTT-aflevering, firmware-updates, opslaan bij
stroomonderbreking, werkelijk bereik en de 24-uursduurtest zijn **niet
gecontroleerd**. Browsertests gebruiken jsdom en testen de geleverde
browsercode, niet alle pagina's op een echte telefoon. Geen volledige
penetratietest, juridische review, AI-diensttoets of uitputtende audit van SDK
en binaire XAPK-inhoud. De geheimenzoektocht gebruikt patronen en kan waarden
missen. Dit is een geautomatiseerde toets door een AI; die kan dingen missen
of verkeerd inschatten en vervangt geen professionele review.
