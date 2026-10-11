# Eén keer via USB flashen; daarna via wifi beheren

Voor jouw **ESP32-WROOM-32, ESP32-D0WD-V3 rev. 3, 4 MB flash**.
Firmware **0.2.0-beta.1** is een softwarematig gecontroleerde bèta; de eerste
test met jouw echte Battery Sense-units blijft nodig. Voor C3/S3/S2 is dit
bestand ongeschikt.

## 1. Eerste flash met ESP32 Flash Download Tool

Het flashpakket wordt opgeslagen onder `firmware/releases/`. Pak de ZIP eerst
uit. Gebruik voor USB het bestand **ctek-esp32-wroom-32-factory.bin**.

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

## 2. Configureer terwijl USB nog aangesloten is

1. Sluit de Flash Tool zodat de COM-poort vrijkomt. Open een seriële terminal
   (bijvoorbeeld PuTTY) op dezelfde COM-poort, **115200, 8 databits, geen
   pariteit, 1 stopbit, geen flow control**. Druk EN/RESET.
2. Noteer de regel **Beheerlogin: admin; uniek wachtwoord: ...**. Dit wordt
   willekeurig op jouw bord gemaakt; er staat geen standaardwachtwoord in Git.
   Bewaar die regel privé. Dezelfde uitvoer noemt het wifi-netwerk
   `CTEK-Setup-xxxxxx`.
3. Verbind je telefoon/laptop met dat setupnetwerk, met dit wachtwoord. Open
   **http://192.168.4.1**. Log in als **admin** met hetzelfde wachtwoord.
   Je telefoon kan dit netwerk zonder internet willen verlaten; blijf ermee verbonden.
4. Vul de **2,4 GHz-wifi** en jouw bestaande MQTT-broker in. De broker is het
   adres van bijvoorbeeld de Mosquitto-add-on, niet een URL van Home Assistant.
   Vul de MQTT-gebruikersnaam/wachtwoord in; laat poort 1883 staan als dat bij
   jouw broker hoort. Sla op. De gateway herstart.
5. Verbind de laptop weer met je gewone netwerk. De seriële uitvoer toont het
   nieuwe gateway-IP. Open dat adres, of `http://ctek-xxxxxx.local` als jouw
   netwerk mDNS ondersteunt. Reserveer bij voorkeur dit IP in de router.
   Als wifi niet lukt, komt het setupnetwerk na circa één minuut terug.

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

1. Open de gatewaypagina en kies **Uitlezen pauzeren en firmware bijwerken**.
   Wacht de actieve poging af en vernieuw de updatepagina.
2. Kies alleen **ctek-esp32-wroom-32-ota.bin** uit het nieuwe pakket.
   De factory-bin wordt hier geweigerd. Upload het bestand en houd de voeding aan.
3. Na validatie herstart de gateway. Controleer versie, sensoren en geslaagde
   metingen. Instellingen en wachtwoorden blijven behouden.
4. Bij afgebroken/mislukte upload blijft de huidige firmware geselecteerd.
   Bij **Annuleren** of na vijf minuten inactiviteit hervat het uitlezen.

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
