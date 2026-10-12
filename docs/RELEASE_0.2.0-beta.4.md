# ESP32 CTEK-gateway 0.2.0-beta.4

Deze diagnose-update helpt onderzoeken waarom een Battery Sense na het
ontgrendelbericht de Bluetooth-verbinding verliest. Bij beta.3 lukken verbinden
en discovery voor plek 2, maar is de verbinding vóór de klok-read verbroken.
Ook na een powercycle is nog geen geslaagde meting gemeld. De oorzaak en de
juistheid van de gebruikte code zijn nog niet bewezen.

## Wijzigingen

- De BLE-log bewaart de eerste disconnectreden, als getal en NimBLE-fouttekst,
  samen met de uitleesstap en de milliseconden sinds het begin van de poging.
- Na het begin van de ontgrendelwrite wordt ook de tijd sinds `unlockstart`
  vermeld. Die tijd omvat de write met response en de daaropvolgende wachttijd.
- De log toont wachttijden en of de client aan het begin daarvan verbonden is.
- De callback bewaart uitsluitend kleine gegevens van zijn eigen client.
  Een later opruimevent schrijft niet in een volgende poging; het eigen
  opruimen wordt niet als onverwachte sensorverbreking geteld.
- Het ontgrendelbericht, de sessievolgorde, wachttijden en sensorinstellingen
  blijven gelijk. Codes, hashes en wachtwoorden worden niet toegevoegd aan logs.

## Via wifi testen

Download de [OTA-bin](https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32/raw/refs/heads/main/firmware/releases/0.2.0-beta.4/ctek-esp32-wroom-32-ota.bin).
Kies op de gateway **Uitlezen pauzeren en firmware bijwerken**, wacht de actieve
poging af en upload dit bestand. Controleer na de herstart **0.2.0-beta.4**.
De OTA-update behoudt de instellingen en het eigen beheerwachtwoord.

Houd telefoon-Bluetooth uit en klik bij plek 2 op **Nu uitlezen**. Wacht totdat
**Bezig met BLE-uitlezing** weg is en vernieuw de pagina; tijdens een poging
hoort de zichtbare log nog bij de vorige afgeronde poging. Download daarna
diagnostics en deel die privé of verwijder de sensoradressen. Een regel met
`BLE-disconnect`, `fase` en `sinds unlockstart` geeft het ontbrekende bewijs.
Een reden betekent niet automatisch dat de verbindingscode verkeerd is.

## Controle en beperkingen

De ESP32-build, hosttests, webbuild en releasetests controleren de software en
flashbestanden. Nieuwe hosttests dekken de eerste reden, behoud na een retry,
timing over klokoverloop en het negeren van normale opruimcallbacks.
Eerdere flashpakketten blijven beschikbaar. Er is nog geen fysieke test van
beta.4; deze versie levert meer diagnosegegevens en garandeert geen werkende
uitlezing. Zie [de installatiehandleiding](FLASHEN.md).
