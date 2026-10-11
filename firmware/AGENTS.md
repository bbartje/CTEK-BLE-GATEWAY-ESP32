# Firmwareonderhoud

De rootinstructies gelden. Deze map bevat firmware voor ESP32-WROOM-32 met 4 MB
flash, met maximaal vier sensoren en één BLE-verbinding tegelijk.

- Houd protocolfeiten en aannames gescheiden; fysiek sensorbewijs ontbreekt
  zolang de gebruiker geen echte sensortest of radiotrace heeft aangeleverd.
- Bij wijzigen van firmware: bouw met PlatformIO, voer de hosttests uit en maak
  het flashpakket opnieuw met `python3 firmware/tools/build_release.py`.
  `npm test` controleert ook of de binary bij de huidige firmwarebronnen hoort.
- Bij een volgende functionele versie: verhoog `kFirmwareVersion` in
  `include/gateway_types.h`, bewaar eerdere releasepakketten en werk handleidingen
  en CI-versiepad bij. De ELF-versie wordt automatisch ingevuld tijdens bouwen.
- Wijzig sensor-klok, meetperiode of historie niet zonder expliciet verzoek.
- Bewaar geen echte codes, wifi-/MQTT-wachtwoorden of ruwe persoonlijke logs in Git.
- Claims over MQTT en OTA op fysieke hardware vereisen die echte test; een build
  of gescripte transporttest mag nooit daarvoor in de plaats worden gezet.
