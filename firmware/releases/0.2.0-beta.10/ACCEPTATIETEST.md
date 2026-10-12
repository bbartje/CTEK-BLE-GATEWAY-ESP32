# Laatste praktijktest en vrijgave

Versie: **0.2.0-beta.10** · Eigenaar/beheerder: **bbartje**.

De softwarecontroles vervangen deze test op een echte ESP32 niet. Vul de
uitslagen zelf in; een leeg vak betekent **niet getest**. Gebruik uitsluitend
ESP32-WROOM-32 / ESP32-D0WD-V3 met 4 MB flash, een stabiele USB-voeding en
2,4 GHz-wifi. Gebruik voor een schone installatie bij voorkeur een reservebord:
een factory-flash wist bestaande instellingen. Bewaar codes en wachtwoorden
privé en maak een back-up van Home Assistant.

Een tweede tester begint bij [TESTER_START.md](TESTER_START.md) en gebruikt een
eigen kopie van [TESTVERSLAG_TEMPLATE.md](TESTVERSLAG_TEMPLATE.md). Dezelfde
controlecriteria gelden op diens eigen hardware. Noteer apart welke controles
de eigenaar en de tweede tester werkelijk hebben uitgevoerd.

## Testvolgorde

| Klaar | Controle | Geslaagd als |
|---|---|---|
| [ ] | Schone USB-installatie volgens [FLASHEN.md](FLASHEN.md) | Factory-image op 0x0 start; setupnetwerk verschijnt; versie is beta.10. |
| [ ] | Eerste wachtwoord | Alleen wachtwoordkeuze beschikbaar; te kort, startwachtwoord en ongelijke herhaling geweigerd; eigen wachtwoord werkt na herstart. |
| [ ] | Eigen wifi en MQTT | Verkeerde wifi/MQTT-invoer geeft een duidelijke melding of herstelroute; met correcte instellingen zijn beide verbonden. |
| [ ] | Eén echte sensor | Eigen code en adrestype werken; live-spanning en actuele temperatuur worden ontvangen; telefoon-Bluetooth uit tijdens gatewaytest. |
| [ ] | Home Assistant | Spanning (twee decimalen als standaard), actuele temperatuur, Bluetooth-signaal in dBm en lage-spanningsstatus verschijnen; de oude dubbele temperatuur-entiteit verdwijnt na OTA. |
| [ ] | Bluetooth-ontvangst | RSSI direct op de kaart en als gewone Home Assistant-sensor; na dezelfde geslaagde uitlezing zijn de dBm-waarden gelijk; bij ontbrekende RSSI geen verzonnen 0 dBm; verouderde ontvangst op de kaart herkenbaar. |
| [ ] | Waarschuwing | Zet tijdelijk de gatewaydrempel boven en daarna onder de gemeten spanning; de status verandert mee. Zet de gewenste drempel daarna terug. Verander niets aan de sensor zelf. |
| [ ] | Herhaalde uitlezingen | Minimaal zes automatische rondes; geslaagde teller neemt toe; Home Assistant krijgt updates; geen onverwachte herstart. |
| [ ] | Meerdere sensoren | Beschikbare sensoren blijven om de beurt bijwerken; noteer hoeveel je echt hebt getest, maximaal vier. |
| [ ] | Sensor buiten bereik | Andere sensoren werken door; na 30 minuten is de verdwenen sensor niet beschikbaar; terug in bereik volgt automatisch herstel. |
| [ ] | Wifi en broker onderbroken | Zet router/wifi en MQTT-broker afzonderlijk tijdelijk uit; na herstel komen metingen terug zonder factory-flash. |
| [ ] | Home Assistant herstart | Entiteiten en verse metingen keren terug; oude metingen worden niet als vers herhaald. |
| [ ] | Lokale tijd | Amsterdam toont de juiste lokale tijd; kies UTC en controleer het verschil; MQTT-tijden blijven dezelfde UTC-tijd voorstellen. |
| [ ] | Nederlands, Engels, Duits en plusknop | Taalkeuze blijft na verversen behouden; alle lege plekken zijn eerst verborgen, ook plek 4 bij drie ingestelde sensoren; iedere klik op plus opent één lege plek; bestaande sensoren blijven zichtbaar; bij vier ingestelde sensoren verdwijnt plus. Technische BLE-logs kunnen Nederlands blijven. |
| [ ] | Compacte sensorkaarten | Naam, live-spanning, Bluetooth-ontvangst en lokale meettijd direct zichtbaar; Details en instellingen standaard dicht; uitklappen toont temperatuur, status, tellers, Nu uitlezen, log en instellingen; verversen klapt weer in. Ontbrekende live-spanning/RSSI en verouderde waarden blijven herkenbaar op de kaart. Een nieuwe plek via plus toont meteen het invulformulier. |
| [ ] | Ingeklapte gatewayinstellingen | Gateway-instellingen begint dicht; openen toont de opgeslagen waarden; inklappen en opnieuw openen bewaart gewijzigde invoer op dezelfde pagina; opslagknop is bereikbaar binnen het formulier. |
| [ ] | Retrostijl en vaste actiebalk | Crème/petrol/oranje, goed leesbaar op desktop en telefoon; bij scrollen blijven vernieuwen, zoeken, diagnostics en firmware bijwerken bereikbaar; toetsenbordfocus zichtbaar; BartjeB / GitHub opent onze repository. |
| [ ] | Herstel uitsluitend via setup-wifi | Binnen twee minuten werkt het startwachtwoord alleen op 192.168.4.1 via het setupnetwerk; alleen wachtwoordherstel beschikbaar, geen instellingen, diagnostics of OTA. Op het thuisnetwerk wordt dezelfde login geweigerd. |
| [ ] | Hersteldeadline | Laat de herstelpagina open tot na twee minuten; opslaan met het startwachtwoord werkt dan niet meer; thuiswifi/MQTT werken door. |
| [ ] | Wachtwoord daadwerkelijk herstellen | Via setup-wifi tijdig een nieuw wachtwoord kiezen; na herstart werkt dit op het thuisnetwerk; wifi/MQTT/sensoren zijn behouden; oud beheerwachtwoord werkt niet meer. |
| [ ] | OTA vanaf bestaande beta | Firmware bijwerken kiezen; huidige poging afronden; beta.10 OTA-bin uploaden; versie en eigen wachtwoord/instellingen blijven correct. Factory-bin wordt geweigerd. |
| [ ] | Afgebroken OTA | Upload afbreken of verkeerd bestand kiezen; oude firmware blijft werken; uitlezen hervat na annuleren of vijf minuten inactiviteit. |
| [ ] | Herstart en duurtest | Voeding normaal uit/aan: instellingen blijven bewaard. Laat daarna minstens 24 uur draaien op de uiteindelijke plek; controleer updates, herstarts en minimumgeheugen. |

Als je minder dan vier sensoren hebt, noteer dat expliciet en laat het scenario
met vier sensoren tijdens de beperkte externe bèta testen. Neem dat scenario
mee vóór een stabiele versie die vier sensoren belooft.

## Uitslag vastleggen zonder privégegevens

- Testdatum, firmwareversie en type ESP32:
- Aantal geteste sensoren, uitleesinterval en duur van de test:
- Bereik/plaatsing, geslaagde en mislukte pogingen:
- Geteste Home Assistant-versie, wifiherstel en OTA-uitkomst:
- Niet geslaagde of niet geteste onderdelen:
- Besluit eigenaar: verder herstellen / beperkte externe bèta starten:

Gebruik fictieve sensornamen. Diagnostics en BLE-logs kunnen Bluetooth-adressen
en meetwaarden bevatten; verwijder die vóór delen. Deel nooit sensorcodes,
wachtwoorden of oude seriële opstartlogs zonder controle.

## Van beta.10 naar 1.0.0

1. **Eigen test:** los mislukte controles eerst op. Wijzigt de firmware na deze
   vastgelegde beta.10-build, maak dan een volgende bètaversie met een nieuw
   flashpakket en test de geraakte functies opnieuw.
2. **Externe bèta:** laat een beperkte groep zelfstandig de handleiding volgen
   op hun eigen ESP32, netwerk, broker en CTEK-sensoren. Verzamel versienummers,
   afgevinkte controles en geanonimiseerde fouten; bewaar hun wachtwoorden niet.
3. **1.0.0:** pas na geslaagde installatie, uitlezingen, Home Assistant,
   verbindingsherstel, wachtwoordherstel en OTA bij de testers. Bouw de
   1.0.0-binaries opnieuw met het nieuwe versienummer; hernoem geen beta-bin.
   Leg resterende beperkingen en de geteste hardware vast in de releasenotities.

Een Git-push met beta.10-code en bestanden bewaart de testkandidaat. Het is geen
verklaring dat de fysieke acceptatietest of stabiele vrijgave al is geslaagd.
Een GitHub-bètarelease met vaste tag maakt de gecontroleerde versie deelbaar;
zie [de vrijgaveprocedure](VRIJGAVE.md).
