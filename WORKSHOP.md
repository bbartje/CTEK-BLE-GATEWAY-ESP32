# Workshop vibe coding: zo werk je in deze omgeving

## 1. Je ontwikkelomgeving

Je werkt in een Codespace: een complete ontwikkelomgeving in je browser. Je app start vanzelf en verschijnt in een voorbeeldvenster. Zie je hem niet? Open onderin het tabblad **Ports** en klik bij poort 3000 op het wereldbolletje.

## 2. Je AI-assistent

Open Codex via het icoon in de zijbalk en log in met het workshop-account. Hoe dat gaat, vertellen we tijdens de workshop.

Lukt inloggen bij Codex niet en heb je zelf een Claude Pro- of Max-abonnement? Dan kun je ook Claude Code gebruiken. Dat staat ook in de zijbalk.

De assistent leest automatisch de instructies in [AGENTS.md](AGENTS.md). Daarin staat hoe hij met jou en met dit project moet omgaan. Je mag dat bestand aanpassen.

## 3. Zo bouw je

- Beschrijf eerst wat je wilt bereiken en voor wie, niet alleen hoe het eruit moet zien.
- Werk aan één functionaliteit tegelijk.
- Moet je kiezen? Vraag om de voor- en nadelen. De assistent praat je graag naar de mond.
- Test na elke wijziging zelf of je app nog doet wat je verwacht.
- Houd bij welke opdrachten goed werken, en noteer fouten, twijfels en vragen.
- Gebruik geen echte persoonsgegevens of vertrouwelijke bedrijfsgegevens.

## 4. Opslaan en terugdraaien

Je werk is pas echt veilig als het in git staat en op GitHub. De assistent doet dat na elke stap die werkt. Je kunt het ook zelf vragen:

- *"Commit en push mijn wijzigingen."*
- *"De laatste wijziging werkt niet. Zet de app terug naar de vorige werkende versie."*

## 5. Als iets niet werkt

- Kopieer de foutmelding, of maak een schermafbeelding, en plak die in de chat.
- Beschrijf wat je verwachtte en wat er gebeurde.
- Is je app verdwenen? Open een terminal (menu **Terminal → New Terminal**) en typ `npm start`.

## 6. Kwaliteitstoets

Laat de assistent je project toetsen en een adviesrapport schrijven:

- **Codex:** typ *"Doe de kwaliteitstoets."*
- **Claude Code:** typ `/kwaliteitstoets`

De assistent vraagt eerst welk niveau je wilt bereiken, wie de app gebruikt en met welke gegevens. Het rapport komt in een bestand `KWALITEITSRAPPORT-<datum>.md`.

## 7. Iemand anders je app laten testen

Open het tabblad **Ports**, klik met de rechtermuisknop op poort 3000 en kies **Port Visibility → Public**. Deel daarna het adres. Dat werkt alleen zolang je Codespace aan staat. Zet het na het testen terug op **Private**.

## 8. Goed om te weten

- Een Codespace stopt na 30 minuten zonder activiteit. Je werk blijft bewaard: open hem weer via [github.com/codespaces](https://github.com/codespaces).
- Je hebt elke maand een beperkt aantal gratis Codespaces-uren. Stop je Codespace als je klaar bent: ga naar [github.com/codespaces](https://github.com/codespaces), klik op **⋯** en kies **Stop codespace**.
- Een Codespace die 30 dagen niet gebruikt is, wordt verwijderd. Wat op GitHub staat, blijft bewaard.
