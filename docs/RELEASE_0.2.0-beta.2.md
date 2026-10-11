# ESP32 CTEK-gateway 0.2.0-beta.2

Deze versie laat je de gateway instellen zonder PuTTY of een seriële terminal.
Voor ESP32-WROOM-32 / ESP32-D0WD-V3 met 4 MB flash; maximaal vier CTEK Battery
Sense-units, om de beurt uitgelezen en via MQTT naar Home Assistant.

Na de eerste factory-flash:

1. Verbind met wifi **CTEK-Setup-xxxxxx**, wachtwoord **CTEK-Start123**.
2. Open **http://192.168.4.1** en log in als **admin**, wachtwoord **CTEK-Start123**.
3. Kies verplicht een eigen wachtwoord (12–63 tekens), vul het twee keer in
   en sla op. Het bord herstart.
4. Verbind opnieuw met hetzelfde setupnetwerk, nu met **je nieuwe wachtwoord**.
   Vergeet zo nodig het opgeslagen wifi-netwerk met het oude wachtwoord.
5. Log in als **admin** met je nieuwe wachtwoord en stel wifi, MQTT en sensoren in.

Wifi/MQTT-instellingen, sensoren en firmware-upload zijn geblokkeerd zolang het
startwachtwoord niet is vervangen. Een bestaand eigen wachtwoord blijft bij een
OTA-update behouden. Het uitleesprotocol is ongewijzigd ten opzichte van beta.1.

Download bij **Assets** `ctek-esp32-wroom-32-factory.bin` voor een eerste flash of
herstel. Flash uitsluitend dit bestand op **0x0**, met **ESP32 / DIO / 40 MHz /
4 MB** en **DoNotChgBin** aangevinkt. Dit wist de eerdere instellingen en geeft
de nieuwe startlogin. PlatformIO is niet nodig om dit bestand te gebruiken.

Gebruik `ctek-esp32-wroom-32-ota.bin` alleen voor updates via de gatewaypagina;
die behoudt je bestaande eigen wachtwoord en instellingen. De ZIP bevat beide
binaries, controlesommen, manifest, handleiding en licenties.

Zie [de installatiehandleiding](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/blob/v0.2.0-beta.2/docs/FLASHEN.md).

**Bèta:** de firmware is gebouwd en softwarematig gecontroleerd. De nieuwe
inlogprocedure, BLE, MQTT en OTA moeten nog op jouw fysieke bord worden getest.
