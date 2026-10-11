# ESP32 CTEK-gateway 0.2.0-beta.3

Deze versie verbetert het zoeken naar Bluetooth-leespunten en de foutmeldingen
bij het uitlezen van CTEK Battery Sense. Voor ESP32-WROOM-32 / ESP32-D0WD-V3
met 4 MB flash. Maximaal vier sensoren worden om de beurt uitgelezen.

## Wijzigingen

- De gateway zoekt alle GATT-punten binnen de CTEK-service daadwerkelijk op
  voordat hij ontgrendelt en uitleest. De vorige logaanroep gebruikte alleen
  NimBLE's nog lege lijst van eerder gevonden punten.
- De BLE-log bevat het aantal punten, hun UUID's en lees-, schrijf- en
  notificatiemogelijkheden. De volledige lijst wordt ook bij de uitlezing
  gebruikt in plaats van uitsluitend per UUID te zoeken.
- Een ontbrekend leespunt, ontbrekende leestoegang, verbroken verbinding en
  mislukte read krijgen afzonderlijke foutmeldingen. De sessie behoudt deze
  details na de begrensde klokretry.
- Een geslaagde ontgrendelwrite meldt expliciet dat de code nog niet bevestigd
  is. Een mislukte verbinding meldt dat de code niet is verstuurd.
- Verbindingsfouten krijgen NimBLE's tekstuele uitleg. De beheerpagina toont
  geen misleidende `BLE-status 0` meer naast een mislukte read.

## Via wifi installeren

1. Download **ctek-esp32-wroom-32-ota.bin** bij deze release.
2. Open de beheerpagina van de ESP32 en kies **Uitlezen pauzeren en firmware
   bijwerken**. Wacht de actieve poging af en vernieuw zo nodig de pagina.
3. Selecteer de OTA-bin, klik **Firmware uploaden** en houd de voeding aan.
4. Open na de herstart de beheerpagina. Controleer **0.2.0-beta.3** en klik bij
   de sensor **Nu uitlezen**. Bekijk daarna de BLE-log of download diagnostics.

De OTA-update behoudt instellingen, sensorcodes en je eigen beheerwachtwoord.
De factory-bin is uitsluitend voor USB-installatie/herstel op **0x0** en wist
instellingen. Eerdere flashpakketten blijven beschikbaar.

## Controle en beperkingen

De ESP32-build, hosttests, webbuild en releasetests controleren de software en
de echte flashbestanden. De sessietests dekken afzonderlijke readfouten,
herstel na een tijdelijke klokfout en een ontbrekende optionele temperatuur.
De releasecontrole valideert ESP32-chip/project/versie, controlesommen,
partitiegrenzen, ZIP-inhoud en overeenkomst met de firmwarebronnen.

**Bèta:** deze update is niet op een fysiek bord getest. De eerdere diagnostics
lieten wel netwerkverbindingen en bij één sensor een Bluetooth-verbinding zien,
maar nog geen geslaagde meting. Volledige discovery kan het zoeken verbeteren
en geeft meer bewijs; een juiste code, leestoegang, radioverbinding en werkende
OTA-herstart moeten met de eigen sensoren worden bevestigd. Er worden geen
sensor-klok, meetperiode of historie-inhoud gewijzigd.

Zie [de installatiehandleiding](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/blob/v0.2.0-beta.3/docs/FLASHEN.md).
