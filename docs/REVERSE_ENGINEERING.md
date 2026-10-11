# Onderzoek CTEK Battery Sense en ESP32

Datum: 11 oktober 2026. Doel: vier monitors met één ESP32-WROOM-32
(ESP32-D0WD-V3 rev. 3, 40 MHz kristal, 4 MB flash), om de beurt uitlezen en via
MQTT naar Home Assistant sturen. De fysieke sensoren zijn nog niet getest.

## Onderzochte bestanden en methode

- Firmware: [bbartje/ctek-ble-ESP32-gateway](https://github.com/bbartje/ctek-ble-ESP32-gateway),
  commit `0cb26cc6b6a7cf0b61241817ef65661e9fabd311`, tag `v0.1.0-beta.8`.
- Referentie: [bbartje/CTEK-BLE](https://github.com/bbartje/CTEK-BLE). Niet als
  bewezen werkend beschouwd; eveneens vergeleken met APK-code.
- `CTEK+Battery+Sense_2.4.6_APKPure.xapk`, SHA-256:
  `fb12ab5de865b2ce31d7f86923354a7553ebb6c7d45e3f90eb84138d2ebebef1`.
- Basis-APK `com.ctek.sba.apk`, SHA-256:
  `8e78d33c51c71416c4318eb1c55f17f9f88192fdb5aa76675943f46f1431f93a`.
- Manifest: pakket `com.ctek.sba`, versie 2.4.6, version code 81,
  Android min SDK 21 / target SDK 36. Extra splits: talen en afbeeldingen.
- Eén `classes.dex`, geen native `.so`-bibliotheken. Gereconstrueerd met JADX
  1.5.6. De app is geobfusceerd: daarom bewijsplaatsen met gegenereerde namen.
  JADX meldde 12 fouten in de totale app; in de aangehaalde protocolmethoden
  stond geen `Method not decompiled`. Type-inferentiewaarschuwingen, vooral in
  SoC-code, blijven een beperking. De app is niet op Android uitgevoerd.
- Originele firmware echt gebouwd met PlatformIO 6.1.19 / espressif32 7.1.3 /
  Arduino ESP32 2.0.17 / NimBLE-Arduino 2.5.1 / PubSubClient 2.8.0 /
  ArduinoJson 7.4.3: **geslaagd**. Statisch RAM 55.540/327.680 bytes,
  app 1.101.093/1.310.720 bytes. Dynamische buffers/taakstapels niet inbegrepen.

Reproduceren: pak `com.ctek.sba.apk` uit de XAPK en gebruik
`jadx -d /tmp/ctek-decompiled com.ctek.sba.apk`. De onderstaande regels horen
bij deze APK-hash en JADX-versie. Decompiled bestanden blijven buiten Git.

## Protocolfeiten uit de APK

BLE betekent Bluetooth Low Energy; GATT verdeelt lees-/schrijfpunten op de
sensor. De app gebruikt actieve verbindingen. Advertenties zijn voor
ontdekking, niet voor het uitlezen van spanning. UUID's: `M/p.java:9–43`,
`M/q.java:9` in de gegenereerde `sources/`-map.

| Functie | UUID | Gegevens |
|---|---|---|
| Service | `812CA8ED-A177-4D74-AEFA-70098C5416AF` | Bevestiging protocol |
| Unlock | `A1D67C01-FE3F-01BF-7D49-7DBDC43B02A6` | Write met response, 16 binaire MD5-bytes |
| Klok | `337EA83E-744B-4B7C-B23F-45BEAAB11DB7` | Vier bytes, little endian, seconden sinds opstarten |
| Periode | `E5A7E9A0-E2D7-4F46-83D1-0E55D2427780` | Eén unsigned byte, minuten |
| Volgende schrijfpositie | `FC3119DA-9B40-414E-A1C4-6C1737718CAE` | Twee unsigned bytes, little endian |
| Historie opvragen | `418D1817-F516-45F1-AB64-433AE596613D` | `[3, start_lo, start_hi, count_lo, count_hi]` |
| Historie ontvangen | `A337B4EB-2544-4F94-87C9-F7732E5FE250` | Notificaties; drie bytes per record |
| Live spanning | `631994F8-FD2E-48BF-AF8F-E86320E17D94` | Read; twee bytes per record |
| Actuele temperatuur | `B5634099-97C4-4F37-ADF3-771246E77754` | Eén signed byte, halve graden Celsius |
| Device Information | `0000180A-0000-1000-8000-00805F9B34FB` | Code, hardware-/softwareversie |

### Ontgrendeling

`M0/g.java:188–219`: MD5 van
`e1da7195eced40e4bc16559e9de73ba4` gevolgd door de ASCII-verbindingscode.
Geen hextekst schrijven: het resultaat zijn 16 binaire bytes.
`NewDeviceActivity.java:110–137` verwijdert witruimte en maakt hoofdletters.
De codeprefix is `40149` (`resources/res/values/strings.xml:43`). De port
accepteert 16 tekens of 11 met die prefix en weigert niet-ASCII-invoer.

Dit is applicatie-authenticatie: in deze route geen OS-pincode, bonding of
cloudtoken. Na de write wacht de app twee seconden (`M/b.java:442–467`) en leest
de klok. Een geslaagde write alleen bewijst geen correcte code. De klok-read
is de volgende bevestiging. Bij status 137 herhaalt de app de klok-read
(`M/b.java:168–202`); de port begrenst dit om niet eindeloos vast te lopen.

### Meetrecord en ringbuffer

`p015h1/a.java:350–372`: modus 2 en 3. De eerste twee unsigned bytes worden
little endian gelezen en gedeeld door 2048. Byte drie is signed en beschrijft
historische temperatuur (`M/b.java:109`: delen door 2 bij opslag).
Voorbeeld `00 64 29` → 12,5 V, 20,5 °C; `EB` als temperatuurbyte → −10,5 °C.
De aparte temperatuur-read rondt Android af (`M/b.java:305–307`); de port
bewaart halve graden en scheidt actueel van historisch.

`p015h1/a.java:76–87` schrijft modus 3, start en **aantal**; de helper krijgt
start/eind en berekent eind minus start. De sensor heeft 30.000 recordposities
(`M/e.java:241–267`, `M/i.java:153–178`). Aanvragen over het einde worden gesplitst.
Cursor is de eerstvolgende schrijfpositie: laatste record is
`(cursor + 30000 - 1) % 30000`. Bij een nieuwe sensor is cursor 0 lege historie;
na rondgaan kan cursor 0 geldige historie zijn. De app onderscheidt dat met
`uptime >= 30000 × periode × 60` (`M/b.java:237–263`). Standaardperiode: vijf minuten.

De klok is geen Unix-tijd. Meetmoment wordt geschat door terug te rekenen vanaf
de telefoontijd met `uptime % (periode × 60)` (`M/b.java:258–263`). De port
mag zonder geldige eigen tijd geen kalenderdatum verzinnen. Bij ontbrekende
historie leest de app live spanning als uptime > 6 s (`M/b.java:265–290`,
`M/e.java:107–115`); ontbreken van historie betekent dus niet automatisch
incompatibiliteit.

### Laadpercentage

SoC (State of Charge, laadpercentage) is geen kant-en-klare BLE-waarde.
`Q/a.java`, `Q/c.java`, `Q/e.java:86–470` berekenen het uit historie,
rust-/laadgedrag en ingestelde accucapaciteit (`N/a.java`). Het model gebruikt
spanningshellingen, afgeleide stroom, hysterese en resets. Groen vanaf 58%,
geel vanaf 35%, rood daaronder (`Q/b.java:23–27`).

De port biedt meetspanning, temperatuur, gegevensleeftijd en instelbare
onderspanningsmelding. Een exact CTEK-laadpercentage vereist een afzonderlijk
getest model en vergelijking met de app; een simpele spanningsomrekening
wordt niet als hetzelfde percentage gepresenteerd. Drempels in Home Assistant
zijn instelbaar en geen universele diagnose van elk accutype.

## Bevindingen oorspronkelijke firmware

Regels verwijzen naar `src/main.cpp` op de bovengenoemde commit.

| Bevinding | Bewijs | Gevolg |
|---|---|---|
| Build slaagt; UUID/salt/deler juist | Build en regels 15–24, 423–443 | Bruikbare protocolbasis |
| Code niet genormaliseerd | 288–304, 423–433 | Spaties/kleine letters breken unlock |
| Cursor 0 overgeslagen | 548–558 | Geen uitlezing na ringbufferomloop |
| Klok/periode-read niet op succes gecontroleerd | 516–534 | Write te snel als unlock behandeld |
| Live-voltagefallback ontbreekt | 541–558 | Sensor zonder historie geeft niets |
| Kapotte notificatielengte geaccepteerd | 437–446 | Alleen volledige veelvouden van drie accepteren |
| Serial-print binnen critical section | 440–451 | Bluetooth-taak kan blokkeren |
| Webhandlers muteren lijst tijdens uitlezen | 301–323, 510–577 | Verwijderen verschuift actieve indices |
| Meteen opnieuw uitlezen | 602–614 | Onnodige verbindingsbelasting |
| Beschikbaarheid alleen gateway | 159–161 | Verdwenen sensor kan online lijken |
| Alleen app-bin; geen wifi-update | README, geen OTA-route | Extra flashbestanden en USB nodig |

De nieuwe protocolkern test normalisatie, lege historie, omloop, signed
temperaturen, strikte pakketlengtes, geschatte meettijden en sensorresets.
De runtime moet eigen snapshots gebruiken en netwerkafhandeling los van
blokkerende BLE-aanroepen laten draaien.

## Haalbaarheid op dit bord

**Technisch aannemelijk; verbinding met jouw sensor nog niet bewezen.**
ESP32 ondersteunt wifi met BLE; de gedeelde radio verdeelt de tijd tussen beide.
Met één sensorverbinding tegelijk is geen vier-verbindingenconfiguratie nodig.
Vier sensoren met enkele bytes per vijf minuten vragen weinig dataverkeer.
De echte beperkingen zijn bereik, verbindingstijd, interferentie, voeding en
sensorrevisie. Een telefoon kan de sensor bezet houden.

Primaire bronnen:

- [Espressif wifi/BLE-coexistence](https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32/api-guides/coexist.html).
- [Espressif meerdere BLE-verbindingen](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/ble/ble-multiconnection-guide.html).
- [CTEK productblad](https://www.ctek.com/storage/C13E0ABE2BD67411C54CC6AA60DED6BB4ED18E31CCE96A745C29EDAAD17C15D0/00000000000000000000000000016281/pdf/media/3818a91e38934325b0e81187054e2336/CTX_BATTERY_SENSE-productsheet-low-UK-EN.pdf): BLE, meetopslag.
- [Espressif Flash Download Tool](https://docs.espressif.com/projects/esp-test-tools/en/latest/esp32/production_stage/tools/flash_download_tool.html).
- [Espressif merge_bin](https://docs.espressif.com/projects/esptool/en/release-v4/esp32/esptool/basic-commands.html).
- [Home Assistant MQTT Sensor](https://www.home-assistant.io/integrations/sensor.mqtt/): beschikbaarheid en vervallen meetwaarden.

Zonder sensor/bord/radiotrace kan de timing en het sensorfirmwaregedrag niet
worden bewezen. Nauwkeurigheid vraagt vergelijking met een multimeter.
Een geslaagde hosttest of firmwarebuild vervangt geen hardwaretest. OTA via
wifi helpt alleen wanneer wifi het bord op de uiteindelijke plek bereikt.

## Verificatie

Hosttests draaien de **productie-C++-kern**, niet een tweede parser in Python.
Compileropties: `-Wall -Wextra -Werror -fsanitize=address,undefined`. De tests
dekken juiste/kapotte records, signed temperatuur, code-invoer, lege historie,
cursor 0, 29.999→0, gesplitste opvraag, periodewijziging, reset, beperkte historie,
kloktijden en overloop van `millis()`. Ook `npm run build` en `npm test` slagen.

De nieuwe runtime **0.2.0-beta.1** is gebouwd voor ESP32-WROOM-32: statisch
RAM 66.264/327.680 bytes en app 1.164.201/1.966.080 bytes. De imageheader bevat
ESP32 chip-ID 0, projectnaam `ctek-ble-gateway` en de juiste versie. Twee
app-partities van 0x1E0000 passen binnen de 4 MB flash. Het echte vrije heap-
geheugen en de taakstapels blijven onderdeel van de fysieke test.

Vier hosttestsuites slagen: protocol, sessie, imageheader en telemetrie.
De transporttest controleert dezelfde sessiecode bij verkeerde code,
unlock-write zonder bevestiging, tijdelijke/blijvende klokfout, ongeldige
metadata, mislukte subscribe, onvolledige historie, ontbrekende temperatuur en
live-fallback. Image-tests weigeren korte headers, andere chips, factory-bin-
headers en andere projectnamen. Telemetrie test de exacte vervalgrens en retries.

De gebruikte SDK is ook nagelezen: NimBLE `readValue()` geeft soms gedeeltelijke
bytes terug bij een fout en vult alleen bij succes de optionele timestamp in.
De adapter controleert daarom die succesbevestiging en logt een leeg/ongeldig
antwoord als fout. `getLastError()` is alleen een verbindingsstatus, geen
betrouwbare GATT-readstatus. De callback blijft zonder blocking IO.
De Arduino WebServer parseert queryargumenten en authenticatieheaders vóór de
uploadcallback, dus de OTA-formuliercode is daar beschikbaar. Arduino Update
roept `esp_ota_set_boot_partition` aan na voltooiing; dit laat ESP-IDF de
image controleren. Deze broncontrole vervangt geen echte OTA-uitvoering.

Het complete flashpakket is gemaakt en gecontroleerd:

| Bestand | Grootte | SHA-256 |
|---|---|---|
| `ctek-esp32-wroom-32-factory.bin` | 1.236.320 bytes | `1f50e3838e47553702480046c610f784c674f3e3ddc119e658a2ae7a4aace704` |
| `ctek-esp32-wroom-32-ota.bin` | 1.170.784 bytes | `e6cab902f7b3d5e53fd831a52ce20f78abdd4d72eb8e3dd3e396c4e555ed3497` |

De factory-bin is voor **0x0**, met componenten op 0x1000 / 0x8000 / 0xE000 /
0x10000. De onafhankelijke Python-validator controleert elke segmentgrens,
ESP-XOR-checksum, ingesloten SHA-256, partitie-MD5, overlap, exacte 4 MB-indeling,
inhoud van componenten op de juiste offsets en een lege configuratiezone.
De OTA-startentry heeft sequence 1 en geldige CRC 0x4743989A: daarmee wordt
bij de eerste boot `ota_0` geselecteerd, waar de applicatie op 0x10000 staat.
Espressif `esptool image_info --version 2` bevestigt chip-ID 0 (ESP32), DIO,
40 MHz, 4 MB, versie/project, geldige checksum en geldige validation hash.
De vijfde suite (`test_release.py`) beschadigt echte imagebytes en partities,
probeert verkeerde versies/projecten/offsets, controleert alle verpakte
bestanden en verlangt dat de firmwarebronhash met het manifest overeenkomt.
**Alle lokale tests en builds slagen.**

De compile time in de appdescriptor komt uit de voorgebouwde Arduino-SDK en
vermeldt 2024; die beschrijft niet de bouwdatum van deze gateway. De werkelijke
releasebouwtijd staat expliciet in `manifest.json` als `built_at_utc`.

Een GitHub Actions-workflow herhaalt build, hosttests, releasechecks en webbuild.
De handleiding bevat één fysieke acceptatietest en een apart OTA-testpad.
Geen sensoren, ESP32-bord of eigen Home Assistant/MQTT-broker beschikbaar hier:
**radioverbinding, meternauwkeurigheid, bereik, werkend MQTT in jouw installatie
en een echte OTA-herstart zijn niet getest**. Dit blijven de open hardwarepunten,
geen bewijs dat de oplossing onmogelijk is.

Gedecompileerde
Android-schermen, advertenties, analytics, cloud-REST en Android-notificaties
zijn niet nodig op het bord; Home Assistant verzorgt grafieken en acties.
