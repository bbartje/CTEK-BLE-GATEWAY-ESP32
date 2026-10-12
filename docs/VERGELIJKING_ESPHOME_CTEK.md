# Wat kunnen we leren van de ESPHome-variant?

Onderzocht op 12 oktober 2026 door vergelijking van onze beta.9-bronnen met
[dtech77pl/esphome-ctek-battery-sense](https://github.com/dtech77pl/esphome-ctek-battery-sense/tree/4104c1870e86dec2e0b1a5d40bf884eceed4934f),
commit `4104c1870e86dec2e0b1a5d40bf884eceed4934f`. Eigenaar van onze gateway:
**bbartje**. Dit is brononderzoek, geen nieuwe fysieke sensortest.

## Historische metingen: bruikbaar vervolg

Beide projecten gebruiken hetzelfde ontgrendelbericht, dezelfde Bluetooth-
service, een aanvraag met startnummer en aantal, en records van drie bytes
(spanning en temperatuur). Onze gateway leest al historie: bij eerste contact
maximaal 12 records, daarna maximaal 48 per sessie. Bij de standaardmeetperiode
van vijf minuten is dat één respectievelijk vier uur. Na een ESP32-herstart
begint het kleine startvenster opnieuw; langere gaten worden niet volledig
ingehaald. De berichten gaan naar het eigen MQTT-onderwerp `/history`.

De [ESPHome-code](https://github.com/dtech77pl/esphome-ctek-battery-sense/blob/4104c1870e86dec2e0b1a5d40bf884eceed4934f/esphome/ctek-battery-sense.yaml)
vraagt ontbrekende records in blokken van maximaal 18 op en bewaart het volgende
recordnummer over herstarts. Dat is een bruikbaar ontwerp voor een latere
importfunctie: kleine blokken, per sensor een hervatpunt en ruimte voor de
andere sensoren tussen de blokken. De oorspronkelijke sensorhistorie hoeft
daarvoor niet te worden gewijzigd.

Blind kopiëren is onverstandig. Hun protocolnotities behandelen de cursor als
oplopende teller en noemen de geheugencapaciteit onbekend. Onze implementatie
gebruikt een ringbuffer van 20.000 records, gebaseerd op het Android-onderzoek.
Overloop en resets moeten daarom op onze sensoren worden gecontroleerd.
Hun timeoutpad schuift bovendien bij nul ontvangen records toch één positie
vooruit; dat kan een ontbrekend record overslaan. Een eigen import moet pas
doorschuiven na een volledig ontvangen blok en rekening houden met mislukte
MQTT-publicaties. Alleen een hervatpunt bewaren garandeert geen aflevering.

## Oude punten zichtbaar maken in Home Assistant

Onze normale Home Assistant-grafieken groeien vanaf ontvangst van actuele
MQTT-metingen. `/history` voegt oude punten niet automatisch toe. De
[Home Assistant-configuratie van de referentie](https://github.com/dtech77pl/esphome-ctek-battery-sense/blob/4104c1870e86dec2e0b1a5d40bf884eceed4934f/homeassistant/ctek_car.yaml)
verzamelt historische gebeurtenissen, verwijdert dubbele tijdstempels en
probeert uurgemiddelden, minima en maxima als langetermijnstatistieken te
importeren. Dat is een aparte historieopslag, geen terugvulling van de gewone
live-sensor.

Voor onze MQTT-gateway is een eigen ontvanger/configuratie nodig. De beschikbare
statistiekimport en versieafhankelijke Home Assistant-syntaxis moeten eerst op
de daadwerkelijk gebruikte installatie worden geverifieerd. Deze vergelijking
bevestigt niet dat de meegeleverde YAML daar direct werkt.

Records bevatten geen eigen datum/tijd. De ESPHome-variant schat de tijd via
veranderingen van de cursor; na terugkomst in bereik kan de afwijking ongeveer
één meetperiode zijn. Onze gateway gebruikt ook de sensorklok en meetperiode.
Onze historie blijft als geschat gemarkeerd. Dubbelen alleen op tijdstempel
verwijderen kan bij opnieuw geschatte tijden onvoldoende zijn; een eigen import
moet ook sensoridentiteit, recordnummer en reset/overloop meewegen.

## Koppelen en efficiëntie

Volgens hun [hardwarewaarnemingen](https://github.com/dtech77pl/esphome-ctek-battery-sense/blob/4104c1870e86dec2e0b1a5d40bf884eceed4934f/docs/PROTOCOL.md)
is de volgorde: verbinden, ontgrendelen, Bluetooth-link versleutelen,
meetnotificaties inschakelen en uitlezen. Te vroeg versleutelen verbrak hun
verbinding; zonder versleuteling mislukten reads met status 15. Ze merken op
dat NimBLE dit automatisch kan afhandelen. Wij gebruiken NimBLE, zij ESP-IDF /
Bluedroid. De volledige uitlezing op onze beta.4 is al fysiek gelukt; extra
versleutelstappen zijn dus geen bewezen noodzakelijke correctie voor ons.

Zij houden verbindingen open en lezen de cursor iedere vijf seconden. Dat
vermijdt opnieuw verbinden, maar houdt de sensor bezet voor de telefoonapp en
past niet direct bij onze keuze voor één verbinding tegelijk en maximaal vier
sensoren. Onze gateway verbindt op het ingestelde interval, leest en verbreekt
weer. Een eerlijke snelheidsvergelijking vereist metingen van sessieduur,
foutpercentage en sensorgebruik; de bronnen bewijzen geen algemene versnelling.

## Voorgestelde volgende stap

Een apart te bouwen historie-import kan beginnen met een handmatige actie per
sensor, blokken van maximaal 18 records, hervatten na herstart en een duidelijk
zichtbare voortgang. Eerst begrensde overdracht en ringbuffergedrag testen,
daarna opslag in Home Assistant kiezen en op de eigen installatie testen.
Zo hoeven nieuwe opslag en verbindingsgedrag niet tegelijk te veranderen.

De referentie waarschuwt dat schrijven van een nieuwe meetperiode de
sensorhistorie wist. Dat nemen we niet over: onze gateway schrijft geen nieuwe
klok of meetperiode en wist geen sensormetingen. Er is voor dit onderzoek geen
externe dienst toegevoegd en geen broncode van het andere project gekopieerd.
