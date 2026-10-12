# CTEK ESP32 Gateway 0.2.0-beta.10

Testkandidaat voor ESP32-WROOM-32 met 4 MB flash. Eigenaar: **bbartje**.
Deze versie richt zich op de beheerinterface. Beta.9 en zijn gepubliceerde
release blijven beschikbaar; beta.10 is een nieuw pakket in de repository,
nog zonder gepubliceerde GitHub-release of fysieke acceptatie.

## Wat is veranderd?

- **Gateway-instellingen** is standaard ingeklapt. Klik op de titel om wifi,
  MQTT, tijdzone, uitleesinterval, waarschuwingsgrens of wachtwoord te wijzigen.
  De opslagknop staat bij het formulier; inklappen bewaart je nog niet opgeslagen
  invoer zolang je op dezelfde pagina blijft.
- Een compacte balk bevat **Vernieuwen**, **Bluetooth zoeken (6 s)**,
  **Diagnostics downloaden** en **Firmware bijwerken**. De balk blijft tijdens
  het scrollen bovenaan staan en past zich aan kleine schermen aan.
- Een retrostijl met crème, petrolgroen, warm oranje en mosterdgeel, duidelijke
  knoppen, afgeronde meetkaarten en een zichtbare toetsenbordfocus.
- Bovenaan staat **BartjeB / GitHub**, een link naar onze eigen repository.
- Taalkeuze, compacte meetkaarten en het toevoegen via `+` blijven beschikbaar.

Het [onderzoek naar de ESPHome-variant](VERGELIJKING_ESPHOME_CTEK.md) beschrijft
mogelijkheden voor grotere historie-import en verschillen in koppelen. Deze
versie wijzigt de Bluetooth-sessie en historiegrenzen niet: 12 records bij
eerste contact, maximaal 48 daarna. Oude punten worden niet automatisch
teruggevuld in de normale Home Assistant-grafieken.

## Bekijken en installeren

Het nieuwe pakket staat in `firmware/releases/0.2.0-beta.10/`; de ZIP staat
één map hoger. Gebruik voor een bestaande gateway uitsluitend
`ctek-esp32-wroom-32-ota.bin`. De factory-bin is voor USB-installatie op `0x0`
en wist instellingen. Volg [FLASHEN.md](FLASHEN.md).

Voor de vormgeving kun je ook [het losse HTML-voorbeeld](VOORBEELD_INTERFACE.html)
in een browser openen. Dit gebruikt de firmwarestijl met fictieve meetwaarden;
de knoppen voeren geen opdrachten uit op een gateway. Het voorbeeld staat los
van het firmwarepakket.

Controleer na de wifi-update versie **0.2.0-beta.10**, je eigen wachtwoord en
opgeslagen sensoren. De instellingen moeten dicht beginnen. Open ze, controleer
de waarden en klap ze zonder opslaan weer dicht. Controleer de actieknoppen
tijdens scrollen, ook op een telefoon. Test de drie talen en controleer een
nieuwe meting en Home Assistant volgens de [checklist](ACCEPTATIETEST.md).

## Controles en grenzen

Browsertests gebruiken de daadwerkelijke HTML-fragmenten en browsercode van
de firmware voor inklappen/openen, behoud van formulierinvoer, de actieknoppen,
taalkeuze, repositorylink en toevoegen van sensoren. De bestaande protocol-,
toegangs- en telemetrietests zijn geslaagd, evenals `npm run build`, alle
15 browsertests/app-tests samen, de ESP32-build en controle van binaries,
controlesommen, ZIP en overeenkomst met de bronnen. De firmware gebruikt
20,6% van het statische RAM en 61,5% van de applicatiepartitie.

Het HTML-voorbeeld is ook met Chromium gecontroleerd op 1280, 390 en 320 pixels
breedte, in Nederlands, Engels en Duits: geen horizontale overloop, openen en
inklappen bewaart invoer, en de actiebalk blijft bij scrollen bovenaan staan.
Dit is een browsercontrole met fictieve gegevens, geen sensor- of OTA-test.

De fysieke beta.10-test, radioverbindingen, Home Assistant en wifi-update
moeten nog worden bevestigd. Het pakket heeft dezelfde netwerk- en
herstelbeperkingen als beta.9; gebruik de eigen vertrouwde thuisomgeving.
