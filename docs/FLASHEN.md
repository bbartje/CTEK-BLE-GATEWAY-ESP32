# Flashen en instellen zonder PlatformIO of PuTTY

Voor jouw **ESP32-WROOM-32, ESP32-D0WD-V3 rev. 3, 4 MB flash**.
Firmware **0.2.0-beta.5** is een softwarematig gecontroleerde bèta; een
test met jouw echte Battery Sense-units blijft nodig. Voor C3/S3/S2 is dit
bestand ongeschikt.

## 1. Eerste flash met ESP32 Flash Download Tool

[Download direct de volledige .bin](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/raw/refs/heads/main/firmware/releases/0.2.0-beta.5/ctek-esp32-wroom-32-factory.bin),
of pak de ZIP uit `firmware/releases/` uit. Gebruik voor USB het bestand
**ctek-esp32-wroom-32-factory.bin**. PlatformIO is niet nodig om te flashen.

1. Sluit het bord met een USB-datakabel aan. Kies in het Espressif-programma
   **ChipType ESP32**, **WorkMode Develop**, **LoadMode UART**.
2. Selecteer het factory-bestand, vink uitsluitend deze rij aan en vul het
   adres **0x0** in. Ook `0x00000000` is goed.
3. Kies **DIO**, **40 MHz**, **32 Mbit / 4 MB** waar deze velden beschikbaar zijn.
   Kies **DoNotChgBin** zodat de gecontroleerde imageheader behouden blijft.
   Recente versies nemen de instellingen uit de bin over.
4. Selecteer jouw COM-poort; start met **115200 baud**. Klik **START**.
   Als verbinden niet lukt: houd **BOOT** ingedrukt, tik kort **EN/RESET** aan,
   start en laat BOOT los zodra het downloaden begint.
5. Wacht op **FINISH** en reset het bord. Deze factory-flash schrijft ook de
   configuratiezone leeg; gebruik hem voor de eerste installatie of herstel.
   Erase is normaal niet nodig en wist alle oude configuratie.

De factory-bin bevat bootloader op 0x1000, partitietabel op 0x8000,
OTA-startdata op 0xE000 en app op 0x10000. Die zijn al samengevoegd; laad geen
extra rijen naast het factory-bestand. Een losse applicatie-bin op 0x0 is fout.

## 2. Configureer via wifi; PuTTY is niet nodig

Na een factory-flash van deze versie zijn de eerste inloggegevens:

| Gegeven | Waarde |
|---|---|
| Wifi-netwerk | `CTEK-Setup-xxxxxx` |
| Startwachtwoord wifi | **CTEK-Start123** |
| Configuratieadres | **http://192.168.4.1** |
| Gebruiker | **admin** |
| Startwachtwoord beheer | **CTEK-Start123** |

1. Laat het bord van stroom voorzien en druk na de flash op EN/RESET.
2. Verbind je telefoon/laptop met `CTEK-Setup-xxxxxx`, met wachtwoord
   **CTEK-Start123**. Je telefoon kan dit netwerk zonder internet willen
   verlaten; kies om ermee verbonden te blijven.
3. Open **http://192.168.4.1**. Als de browser om inloggen vraagt: gebruiker
   **admin**, wachtwoord **CTEK-Start123**.
4. Je ziet **Kies eerst je eigen wachtwoord**. Kies 12–63 tekens: letters,
   cijfers en gewone leestekens, zonder spaties of accenten. Vul het twee keer
   in en sla op. Het startwachtwoord mag niet als eigen wachtwoord worden gebruikt.
   Wifi/MQTT, sensoren en firmware-upload blijven tot deze stap geblokkeerd.
5. Het bord herstart en het setupnetwerk verdwijnt even. Verbind daarna met
   hetzelfde `CTEK-Setup-xxxxxx`, maar met **je nieuwe wachtwoord**. Als je
   telefoon/laptop nog het startwachtwoord gebruikt: vergeet het netwerk in de
   wifi-instellingen en verbind opnieuw.
6. Open opnieuw **http://192.168.4.1** en log in als **admin** met je nieuwe
   wachtwoord. Ook de browser kan nog de oude login onthouden en opnieuw om
   de juiste gegevens vragen. Bewaar je eigen wachtwoord privé.
7. Vul de **2,4 GHz-wifi** en jouw bestaande MQTT-broker in. De broker is het
   adres van bijvoorbeeld de Mosquitto-add-on, niet een URL van Home Assistant.
   Vul de MQTT-gebruikersnaam/wachtwoord in; laat poort 1883 staan als dat bij
   jouw broker hoort. Sla op. De gateway herstart.
8. Verbind de laptop weer met je gewone netwerk. Open `http://ctek-xxxxxx.local`
   als jouw netwerk dit ondersteunt (`xxxxxx` is dezelfde suffix als bij setup).
   Anders vind je het IP-adres in de lijst met verbonden apparaten van je router;
   de apparaatnaam is `ctek-xxxxxx`. Reserveer bij voorkeur dit IP in de router.
   Als wifi niet lukt, komt het setupnetwerk na circa één minuut terug, met
   **je eigen wachtwoord**.

Bij een OTA-update vanuit een eerdere versie blijft je bestaande eigen
beheerwachtwoord behouden; het startwachtwoord wordt dan niet opnieuw ingesteld.
Als je het oude wachtwoord niet kunt uitlezen, installeer deze nieuwe factory-bin
op **0x0**. Dat wist de oude instellingen en begint met de bovenstaande stappen.

## 3. Eén sensor testen en daarna de andere drie toevoegen

1. Sluit de CTEK-app volledig om een bestaande Bluetooth-verbinding vrij te
   maken. Begin bij één sensor die dichtbij de ESP32 staat.
2. Klik **Zoek Bluetooth-apparaten**, wacht zes seconden en vernieuw de pagina.
   Kopieer het adres en het adrestype naar plek 1. De scan noemt kandidaten;
   de echte CTEK-service wordt tijdens verbinden gecontroleerd.
3. Geef een naam en de code van het sensorlabel: 16 tekens, of de 11 tekens
   zonder prefix `40149`. Sla op. Een verkeerde code kun je op de webpagina
   corrigeren; daar is geen reflash voor nodig.
4. Vernieuw na 15–60 seconden. Bij een gezonde verbinding zie je spanning,
   temperatuur en **Meting ontvangen**. Een nieuwe sensor zonder historie
   gebruikt live spanning. Vergelijk bij een stilstaande accu de spanning met
   de CTEK-app of een multimeter; sluit de app daarna weer.
5. Controleer in Home Assistant dat de MQTT-integratie actief is. Onder
   **Instellingen → Apparaten en diensten → MQTT** verschijnt de sensor met
   spanning, twee temperaturen, signaalsterkte en **Lage spanning**. De twee
   temperaturen horen bij verschillende momenten en mogen verschillen.
   De standaardweergave is twee decimalen voor spanning en één voor temperatuur.
   Een eerder handmatig gekozen aantal decimalen in Home Assistant blijft gelden.
6. Voeg sensoren 2–4 toe. Controleer over minstens twee uitleesrondes dat elke
   sensor meer dan één geslaagde uitlezing krijgt. De standaardronde is eens
   per vijf minuten per sensor, geen vier blijvend open verbindingen.

Een fout geeft geen nulmeting. De oude waarde wordt na 30 minuten onbeschikbaar.
Gebruik **Nu uitlezen** voor een nieuwe poging. **Diagnostics downloaden**
bevat stappen en ruwe meetmetadata, geen codes/wachtwoorden. Verwijder wel
sensoradressen vóór openbaar delen. USB is voor deze controles niet meer nodig.

## 4. Controle op de definitieve plek

Verplaats het bord pas nadat sensor 1 en MQTT werken. Test op de definitieve
plek met motorkappen/garagedeuren in hun normale stand. Controleer wifi en alle
vier BLE-signalen gedurende enkele rondes. Afstand alleen zegt weinig door
metaal en interferentie. Houd een stabiele USB-voeding; dit project sluit de
ESP32 niet rechtstreeks op 12 V aan.

Voer desgewenst één bereiktest uit: maak één sensor onbereikbaar en controleer
dat de andere blijven verversen. Na 30 minuten moet die ene sensor
onbeschikbaar worden. Breng hem terug en controleer herstel zonder herstart.

## 5. Volgende firmware via wifi

Voor een update vanaf beta.1, beta.2, beta.3 of beta.4:
[download de OTA-bin van beta.5](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/raw/refs/heads/main/firmware/releases/0.2.0-beta.5/ctek-esp32-wroom-32-ota.bin).

1. Open de gatewaypagina en kies **Uitlezen pauzeren en firmware bijwerken**.
   Wacht de actieve poging af en vernieuw de updatepagina.
2. Kies alleen **ctek-esp32-wroom-32-ota.bin** uit het nieuwe pakket.
   De factory-bin wordt hier geweigerd. Upload het bestand en houd de voeding aan.
3. Na validatie herstart de gateway. Controleer versie, sensoren en geslaagde
   metingen. Instellingen en wachtwoorden blijven behouden.
4. Bij afgebroken/mislukte upload blijft de huidige firmware geselecteerd.
   Bij **Annuleren** of na vijf minuten inactiviteit hervat het uitlezen.

Controleer na deze update dat de pagina **0.2.0-beta.5** meldt. De gateway
verstuurt de sensorinstellingen opnieuw naar Home Assistant: spanning standaard
met twee decimalen en beide temperaturen met één decimaal. Instellingen die je
zelf in Home Assistant hebt gekozen gaan voor. De meetwaarden en bestaande
sensoridentiteiten blijven behouden; opnieuw toevoegen is niet nodig.

Klik bij een
sensor **Nu uitlezen** en bekijk de **BLE-log**. Daar staat nu `GATT-discovery`
met het aantal gevonden punten, gevolgd door de UUID's en hun mogelijkheden.
Een punt dat ontbreekt, een punt met `read=0` en een echte readfout krijgen
verschillende meldingen. `unlock write: OK` betekent dat het bericht is
verstuurd; de code is dan nog niet bevestigd. Bij een verbindingsfout is de
code helemaal niet verstuurd. Beta.4 voegt `BLE-disconnect` toe met de reden,
uitleesstap en timing van een verbreking vóór het eigen opruimen. Deze reden
bewijst op zichzelf niet dat de code onjuist is. Bij **Bezig met BLE-uitlezing**
staat er nog de log van de vorige afgeronde poging: wacht tot de poging klaar
is en vernieuw de pagina. Als uitlezen nog mislukt, download dan de nieuwe
diagnostics en deel die zonder adressen of andere privégegevens.

Een foutenloze upload bewijst niet dat een nieuwe programmalogica goed werkt.
Er is geen automatische bootrollback; als een latere versie niet meer start
of wifi niet meer werkt kan USB-herstel nodig blijven.

## 6. Waarschuwing in Home Assistant

Maak in **Instellingen → Automatiseringen** een automatisering voor het
ontdekte **Lage spanning**-signaal. Bijvoorbeeld: status **aan** gedurende
15 minuten → stuur een melding naar je telefoon. Kies je eigen drempel op de
gateway; 12,2 V is alleen de startinstelling, geen universele accudiagnose.
Maak daarnaast een melding wanneer een sensor langere tijd onbeschikbaar is.
Zo kan ontbrekende data niet stilzwijgend als een gezonde accu worden behandeld.

Deze versie levert geen exact CTEK-laadpercentage. De normale HA-grafiek begint
bij de ontvangst van metingen; oude historie op `/history` wordt niet automatisch
teruggeschreven in de grafiek. Meer uitleg: [firmware/README.md](../firmware/README.md).
