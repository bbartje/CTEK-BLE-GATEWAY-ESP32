# ESP32 CTEK-gateway 0.2.0-beta.5

De spanning verscheen in Home Assistant afgerond op hele volts, terwijl de
gateway de meetwaarde al met decimalen verstuurt. Deze versie geeft bij de
automatische MQTT-sensorinstellingen het gewenste aantal decimalen mee.

## Wijziging

- Spanning: standaard twee decimalen, bijvoorbeeld **12,54 V**.
- Historische en actuele temperatuur: standaard één decimaal, bijvoorbeeld
  **11,5 °C**.
- Bluetooth-signaalsterkte: een geheel getal.

De gateway gebruikt hiervoor `suggested_display_precision` in MQTT Discovery,
zoals beschreven in de [Home Assistant MQTT-sensordocumentatie](https://www.home-assistant.io/integrations/sensor.mqtt/).
De volledige numerieke meetwaarden op `/state` en `/history` blijven beschikbaar.
Het aantal getoonde decimalen verandert de meetnauwkeurigheid van de sensor niet.
Een [eigen decimaalinstelling in Home Assistant](https://developers.home-assistant.io/blog/2023/02/08/sensor_presentation_rounding/)
gaat voor op het voorstel van de gateway.

## Via wifi installeren en controleren

1. Download [de OTA-bin van beta.5](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/raw/refs/heads/main/firmware/releases/0.2.0-beta.5/ctek-esp32-wroom-32-ota.bin).
2. Kies op de gateway **Uitlezen pauzeren en firmware bijwerken**, wacht de
   actieve poging af en upload de OTA-bin.
3. Controleer na de herstart **0.2.0-beta.5**. Instellingen en eigen
   beheerwachtwoord blijven behouden.
4. De gateway publiceert de bijgewerkte sensorinstellingen opnieuw zodra MQTT
   verbonden is. Controleer spanning met twee decimalen en temperatuur met
   één decimaal in Home Assistant. Bestaande sensoridentiteiten blijven gelijk.
   Een handmatige instelling blijft voorgaan; je hoeft de sensor niet te verwijderen.

## Controle en grenzen

De ESP32-build, hosttests, `npm run build` en `npm test` controleren de software
en de geleverde flashbestanden. De eerdere beta.4-test bevestigt een volledige
uitlezing van plek 3; de gebruiker meldt ook spanning en temperatuur in Home
Assistant. De nieuwe standaardweergave van beta.5 moet nog op de eigen
installatie worden gecontroleerd. De Bluetooth-uitleesvolgorde is gelijk.
Eerdere releasepakketten blijven beschikbaar.
