# ESP32-firmware

Voor **ESP32-WROOM-32 / ESP32-D0WD-V3, 4 MB flash**, geen ESP32-S2/C3/S3-image.
Eén BLE-verbinding tegelijk; maximaal vier ingestelde sensoren. De bestaande
firmware is onderzocht als referentie. Deze runtime is opgedeeld om het
protocol en foutgedrag onafhankelijk van fysieke hardware te kunnen testen.

## Bouwen en testen

Installeer PlatformIO Core 6.1.19. Python 3 en `g++` zijn nodig voor hosttests.
Vanaf de hoofdmap:

```sh
pio run --project-dir firmware -e esp32-wroom-32
npm run build
npm test
```

Complete release bouwen (inclusief bootloader/partities samenvoegen en controles):

```sh
python3 firmware/tools/build_release.py
```

De nieuwste binaries staan in `releases/0.2.0-beta.9/`; de ZIP staat één niveau
hoger. `test_release.py` controleert de echte bestanden, beschadigde kopieën,
partitiegrenzen, overlap, controlesommen, ZIP en de hash van de firmwarebronnen.
Na bronwijzigingen eerst opnieuw het pakket bouwen; daarna `npm test`.

De dependencies in `platformio.ini` staan vast: espressif32 7.1.3,
Arduino ESP32-package 4.20017.260907+sha.dcc1105b (core 2.0.17),
esptool-package 2.41100.260830, NimBLE-Arduino 2.5.1,
PubSubClient 2.8.0 en ArduinoJson 7.4.3. De laatste drie bestonden al in de
beoordeelde firmware. mDNS, WebServer, Update en Preferences horen bij Arduino.
De frameworkinstellingen schakelen wifi/BLE-coexistence in. De NimBLE-host en
uitleestaak draaien op core 1; wifi/controller zijn in dit SDK op core 0.
De hostlimiet is één BLE-verbinding; de voorgebouwde controller ondersteunt drie.
Omdat het programma er één gebruikt, ontstaat geen controller/hostoverschrijding.

`tools/build_metadata.py` vult alleen projectnaam/versie in de gelinkte ELF-
descriptor in. De SDK wordt niet gewijzigd. esptool maakt daarna de bin met
de normale checksum en SHA. De OTA-headercontrole kan zo een andere app weigeren.

## Indeling

- `ctek_protocol`: normalisatie, ruwe bytes, klok, ringbuffer en historietijden.
- `ctek_session`: uitleesvolgorde via een vervangbaar transport; hosttests gebruiken
  een script voor de BLE-antwoorden en testen dezelfde productiecode.
- `ble_worker`: NimBLE-adapter in een afzonderlijke FreeRTOS-taak.
- `ble_diagnostics`: bewaart de eerste verbreekreden, uitleesstap en timing.
- `main`: eerlijk over vier plekken verdelen, rapporten verwerken en fout-backoff.
- `config_network`: NVS-configuratie, setup-AP, wifiherstel, mDNS, tijd.
- `mqtt_gateway`: Discovery, availability, state, historie en gatewaydiagnostics.
- `web_gateway`: configuratie, invoercontrole, logs, beheerlogin en OTA-upload.
- `telemetry` / `firmware_image`: gedeelde en geteste beslissingen over versheid,
  spanning, tijd, foutinterval en toegestane updateheaders.
- `admin_access`: startwachtwoord, invoercontrole en verplichte eerste wijziging.
- `web_ui`: leesbare HTML en browsercode voor taalkeuze en sensorplekken;
  dezelfde code wordt met jsdom (een nagebootste browser) getest.

De bouwstap `tools/webserver_security.py` begrenst multipart-scheidingstekens
op 70 tekens vóór de uploadparser geheugen op de stack reserveert. Dit is een
gerichte correctie voor [CVE-2026-42854](https://github.com/espressif/arduino-esp32/security/advisories/GHSA-8cmm-3887-r32j)
in Arduino 2.0.17. PlatformIO compileert een kopie onder `.pio/build/`;
de geïnstalleerde SDK blijft intact. Bij een onverwachte parserwijziging
stopt de build, zodat de correctie opnieuw beoordeeld moet worden. De hosttest
controleert de grenzen 0, 1, 70, 71 en 12.000 vóór geheugenreservering.

Alle NimBLE-aanroepen lopen via één worker. Opdrachten bevatten een kopie van
adres/code en een versienummer voor de sensorplek. Een tussentijdse wijziging
kan een oud antwoord niet aan een nieuwe sensor koppelen. De callback kopieert
begrensde data; geen web-, MQTT-, String- of Serial-aanroepen daarin.
Verbindingspoging maximaal 7 s; notificaties maximaal 15 s per aanvraagdeel.
Een individuele GATT-procedure heeft NimBLE's eigen 30 s timeout; een sessie kan
bij slecht radiocontact langer duren. De hoofdtaak blijft intussen beschikbaar.
MQTT-TCP-pogingen zijn begrensd op 1,5 s (DNS kan extra tijd kosten).

Vanaf beta.3 roept de worker `getCharacteristics(true)` aan voor volledige
GATT-discovery binnen de CTEK-service. Zonder `true` geeft NimBLE 2.5.1 alleen
eerder gevonden punten terug. De log bevat aantal, UUID en lees-/schrijf- en
notificatiemogelijkheden. De adapter meldt ontbrekende punten en `read=0`
afzonderlijk van een echte mislukte read. De sessie behoudt deze foutdetails
na de begrensde klokretry. NimBLE-foutteksten zijn ingeschakeld; de beheerpagina
toont alleen een niet-nul verbindingsstatus, als verbindingsstatus. Een
ontgrendelwrite geldt nooit als bewijs van een juiste code. De softwaretests
dekken de foutdetails, herstel na een tijdelijke fout en de optionele
temperatuur-read; radio- en GATT-gedrag van deze update vereisen een sensortest.

Vanaf beta.4 bewaart `onDisconnect` de eerste reden, fase en verstreken
milliseconden. De worker voegt die vóór het eigen opruimen toe als
`BLE-disconnect`. Tijd sinds `unlockstart` begint vóór de write met response;
die tijd omvat dus ook de write zelf. De bestaande sessievolgorde en wachttijden
blijven gelijk. De log vermeldt ook de wachttijden en verbindingsstatus daarbij.
Reden en fase zijn waarnemingen, geen bewijs dat een code onjuist is.

Iedere client bezit zijn eigen callback en diagnosegegevens; NimBLE verwijdert
die pas wanneer de client werkelijk wordt verwijderd. Een callback gebruikt
geen rapportpointer en kan daardoor niet in de volgende sensorlog schrijven.
Worker en callback beschermen de kleine diagnosegegevens met een kritieke
sectie; tekstvorming gebeurt uitsluitend in de worker. Vóór `deleteClient`
wordt het eigen opruimen gemarkeerd, zodat de normale disconnect niet als
sensorfout wordt vastgelegd. Hosttests controleren de eerste reden, behoud na
retry, timing bij `millis()`-overloop en het negeren van opruimcallbacks.

## MQTT-contract

Vanaf beta.5 bevat de sensor-Discovery `suggested_display_precision`: 2 voor
spanning en 1 voor temperatuur. Beta.6 publiceert één live spanningsensor,
één actuele temperatuursensor en de bestaande lage-spanningsstatus. De gateway
verwijdert de oude Discovery-entiteit voor `current_temperature`;
de bestaande `temperature`-entiteit vertegenwoordigt voortaan de actuele
temperatuur. De fabrikant in Home Assistant is `BartjeB`. Numerieke waarden
worden niet extra afgerond; een handmatige decimaalinstelling in Home Assistant
gaat voor op het voorstel. Zie de [MQTT-sensordocumentatie](https://www.home-assistant.io/integrations/sensor.mqtt/)
en [uitleg over weergaveprecisie](https://developers.home-assistant.io/blog/2023/02/08/sensor_presentation_rounding/).

Beta.9 herstelt de `rssi`-sensor als `Bluetooth-signaal`, met eenheid `dBm`,
apparaatklasse `signal_strength`, nul decimalen en hetzelfde `unique_id` als
vóór beta.6. De sensor staat bij de gewone sensoren. Hij is tijdens beta.6–8
uit Discovery verwijderd, terwijl de RSSI in het statebericht bleef staan.
De waarde hoort bij de laatste geslaagde uitlezing. NimBLE's foutwaarde `0`
en de controllerwaarde `127` voor onbekend worden als `null` verstuurd.

`<gateway>` = wifi-MAC zonder dubbele punten, kleine letters.
`<sensor>` = sensor-MAC zonder dubbele punten, kleine letters.

| Onderwerp | Inhoud | Retained |
|---|---|---|
| `ctek_gateway/<gateway>/availability` | `online` / `offline`, met Last Will | Ja |
| `ctek_gateway/<gateway>/<sensor>/availability` | Verse meting aanwezig | Ja |
| `ctek_gateway/<gateway>/<sensor>/state` | Laatste meting, JSON | Nee |
| `ctek_gateway/<gateway>/<sensor>/history` | Eén historisch record per bericht | Nee |
| `ctek_gateway/<gateway>/diagnostics` | Versie, geheugen, wifi, tijd | Nee |
| `homeassistant/sensor/ctek_<gateway>_<sensor>/<veld>/config` | Discovery | Ja |
| `homeassistant/binary_sensor/ctek_<gateway>_<sensor>/low_voltage/config` | Discovery waarschuwing | Ja |

State bevat live `voltage` en `temperature` (de actuele temperatuur),
`low_voltage`, `low_voltage_threshold`, `rssi`, `source`, `sample_age_seconds`,
`last_successful_read`, `sample_period_minutes`, `sensor_uptime_seconds`,
`cursor`, `history_omitted`, `sensor_reset_detected`, `sample_time` (Unix-seconden
of null) en `sample_time_estimated`. Tijdstempels zijn Unix epoch in UTC; Home
Assistant toont ze in zijn ingestelde tijdzone. Bij een ontbrekend live meetpunt
is de bijbehorende livewaarde **null**, nooit een verzonnen nul.
Bij ontbrekende live-spanning is ook `low_voltage` null. Home Assistant toont
dan onbekend, via de [MQTT binary-sensordefinitie](https://www.home-assistant.io/integrations/binary_sensor.mqtt/).
Een historisch record bepaalt geen actuele waarschuwing. De beheerpagina
benoemt historische spanning uitdrukkelijk als historisch.
Bij ontbrekende RSSI is ook `rssi` null; een startwaarde of fout wordt niet als
een ontvangststerkte van 0 dBm gepresenteerd.

Voorbeeld met fictieve gegevens:

```json
{"voltage":12.54,"temperature":21.0,"low_voltage":false,
 "low_voltage_threshold":12.2,"rssi":-68,"source":"live",
 "sample_age_seconds":0,"last_successful_read":1791792000,
 "sample_period_minutes":5,"sensor_uptime_seconds":960,"cursor":3,
 "history_omitted":0,"sensor_reset_detected":false,
 "sample_time":1791792000,"sample_time_estimated":false}
```

Home Assistant vereist online-status van zowel gateway als sensor en gebruikt
`expire_after: 1800`. State wordt niet retained verzonden: anders kan een oude
waarde na herstart onterecht weer vers lijken. Na MQTT-herverbinding of het
Home Assistant birthbericht (`homeassistant/status = online`) worden Discovery
en beschikbare verse metingen opnieuw verstuurd.

## Historie en betrouwbaarheid

Bij eerste contact de laatste maximaal 12 records (standaard één uur).
Daarna ontbrekende records, begrensd tot 48 per sessie (standaard vier uur).
Bij langere onderbreking meldt `history_omitted` hoeveel records buiten het
venster vielen. Uptime/period/cursor bewaken continuïteit; een reset of verandering
start een nieuw venster. Geen nieuwe records betekent: laatste opnieuw uitlezen
voor versheid, zonder nieuwe historie te publiceren.

De `/history`-berichten hebben geschatte meettijden. **Home Assistant MQTT Sensor
voegt die niet automatisch als oude punten toe aan Recorder.** De normale
grafiek bouwt op vanaf de ontvangst van `/state`. Voor backfill/archivering is
een afzonderlijke ontvanger nodig; die wordt niet als externe dienst toegevoegd.

Publicaties gebruiken QoS 0. De sensor bewaart historie; de ESP32 bewaart alleen
de laatste sessie per sensor in RAM en configuratie in NVS. Bij langdurige
brokeruitval kunnen historie-/stateberichten verloren gaan; geen gegarandeerde,
verliesloze archivering. Er wordt niet iedere vijf minuten naar ESP-flash
geschreven. Na reset begint de beperkte historielezing opnieuw.
Verwijderen terwijl MQTT offline is kan retained Discovery in de broker
achterlaten; verwijder die oude entiteiten/config-topics in Home Assistant of
verwijder de sensor wanneer de broker verbonden is.

## Beheer en grenzen

Default pollinterval 300 s; instelbaar 60–900 s. Bij mislukking retry na
30/60/120/240/300 s. Bij vier verdwenen sensoren wordt ieder om de beurt
geprobeerd; geen eindeloze poging voor dezelfde sensor.

De beheerpagina ondersteunt Nederlands, Engels en Duits. De browser bewaart de
keuze lokaal. Vanaf beta.7 zijn alleen ingestelde sensoren direct zichtbaar;
alle lege plekken blijven verborgen totdat je op `+` klikt. Iedere klik opent
één lege plek, ook vóór het opslaan van de eerste sensor. Bij vier ingestelde
sensoren verdwijnt de plusknop. Niet opgeslagen plekken zijn na verversen weer
verborgen.
Vanaf beta.8 toont een ingestelde sensor een compacte kaart met naam,
spanning en tijdstip van de laatste geslaagde meting. Beta.9 voegt de
Bluetooth-ontvangststerkte in dBm toe aan deze eerste weergave. Bijvoorbeeld:
−60 dBm is sterker dan −90 dBm. Ontbrekende RSSI is niet beschikbaar; een
verouderde waarde wordt als verouderd gemarkeerd. Temperatuur, status,
tellers, `Nu uitlezen`, BLE-log, wijzigen en verwijderen staan onder
`Details en instellingen`, standaard ingeklapt. Ontbrekende live-spanning wordt
op de kaart als niet beschikbaar getoond; een verouderde waarde blijft als
verouderd herkenbaar. De meettijd volgt de gekozen lokale tijdzone.

Na iedere herstart met opgeslagen thuiswifi verschijnt twee minuten een herstel-AP met het
publieke startwachtwoord `CTEK-Start123`; normale wifi-, BLE- en MQTT-start
worden niet vertraagd en instellingen worden niet gewist. In dit venster werkt
ook `admin` met het startwachtwoord, uitsluitend voor de wachtwoordherstelpagina
op de setup-interface en vanuit het setup-subnet. Instellingen, diagnostics en
OTA blijven voor deze login geblokkeerd. Via de thuiswifi is het openbare
wachtwoord ongeldig. Na herstellen blijven andere instellingen bewaard.
Iedereen binnen radiobereik kan tijdens het venster het wachtwoord vervangen;
de eigenaar heeft deze herstelkeuze voor deze bètatest bewust geaccepteerd.
De openbare herstel-login verloopt op een vaste deadline, ook als een browser
verbonden blijft. Bij een latere opgeslagen-wifi-uitval kan de bestaande
setup-AP terugkomen met het eigen beheerwachtwoord.

De ingestelde tijdzone is een regio-ID en standaard `Europe/Amsterdam`; de
firmware gebruikt bijbehorende POSIX-regels voor zomer-/wintertijd. De selectie
bevat ook UTC, Londen, Lissabon, vier gangbare Noord-Amerikaanse zones, India,
China, Singapore, Tokio, Sydney en Auckland. De webpagina toont lokale tijd;
MQTT blijft UTC epoch versturen zodat Home Assistant lokaal kan formatteren.

Bij lege configuratie gebruikt het bord het publieke startwachtwoord
`CTEK-Start123`, voor WPA2-setup en HTTP-login als `admin`. Alleen de pagina om
een eigen wachtwoord te kiezen is dan beschikbaar. Andere beheerpagina's,
POST-acties en de multipart OTA-upload blokkeren normaal beheer totdat een
ander geldig wachtwoord van 12–63 zichtbare ASCII-tekens is opgeslagen. Ook BLE
en MQTT worden pas daarna gestart; de eerste setup is uitsluitend een accesspoint,
zonder verbinding met opgeslagen thuiswifi. Het wachtwoord moet twee keer worden
ingevuld en mag niet gelijk zijn aan het startwachtwoord.

De overgang gebruikt één NVS-waarde (`admin_pw`), zonder aparte vlag die bij
stroomuitval uit de pas kan lopen. Bij een mislukte NVS-write wordt de toegang
in RAM niet vrijgegeven. Na opslaan herstart het bord en heeft het setupnetwerk
het nieuwe wachtwoord. Bestaande eigen wachtwoorden uit beta.1 blijven bij OTA
geldig; een factory-flash wist instellingen en begint opnieuw met het publieke
startwachtwoord. PuTTY is voor installatie niet nodig. USB-serieel kan nog als
lokale diagnostiekmogelijkheid dienen; de opstartuitvoer toont geen eigen
beheerwachtwoord meer. Oude firmwareversies konden dit wel tonen: deel oude
opstartlogs daarom niet zonder ze te controleren.

HTTP Basic-login en een formuliercode beschermen configuratie en uploads.
Wachtwoorden/codes worden nooit teruggestuurd in formulieren of
diagnostics. Diagnostics bevatten wel sensoradressen; anonimiseer vóór delen.
HTTP, MQTT en NVS zijn niet versleuteld. Gebruik een vertrouwd thuisnetwerk.

OTA pauzeert BLE, wacht de actieve opdracht af, controleert ESP32-chip/project,
limiteert de lengte en laat Update/ESP-IDF de complete image controleren.
Alleen de andere app-partitie wordt geschreven; configuratie blijft behouden.
Er is **geen automatische terugval bij een programmafout na een succesvolle
updateboot**. Een afgebroken upload schakelt de actieve firmware niet om.

Niet overgezet: Android-schermen, cloud/analytics, notificatieservice,
CTEK's berekende SoC en volledige driemaandengeschiedenis. Home Assistant doet
grafieken en meldingen. Er wordt niet geschreven naar sensor-klok, meetperiode,
accucapaciteit of historie-inhoud; alleen unlock, CCCD en recordopvraag.

Fysieke acceptatie blijft nodig voor GATT-toegang, bereik, voeding, meerdere
sensoren, OTA en MQTT met de eigen broker. Zie [FLASHEN.md](../docs/FLASHEN.md).
