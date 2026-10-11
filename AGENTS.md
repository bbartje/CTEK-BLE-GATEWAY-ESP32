# Instructies voor de AI-assistent

Dit project is gestart tijdens een workshop vibe coding. De gebruiker is meestal geen programmeur.

## Communicatie

- Schrijf in het Nederlands, in gewone taal. Leg vaktermen kort uit.
- Vertel na elke wijziging in een paar zinnen wat je hebt veranderd en hoe de gebruiker het kan bekijken of testen.
- Is een verzoek onduidelijk, of zijn er meerdere redelijke keuzes? Stel dan eerst een vraag, of leg de opties met voor- en nadelen voor. Vul niet zelf in wat de gebruiker niet heeft gezegd.
- Wees eerlijk. Zeg het als een idee onverstandig is, als iets niet werkt of als je iets niet zeker weet. Praat de gebruiker niet naar de mond.

## Werkwijze

- Werk in kleine stappen: één functionaliteit tegelijk.
- Vraag eerst toestemming voor grote wijzigingen, zoals een nieuwe bibliotheek, een andere opbouw van het project, een database of een externe dienst.
- Controleer na elke wijziging dat `npm run build` en `npm test` slagen.
- Commit na elke stap die werkt, met een duidelijk commitbericht in het Nederlands, en push naar GitHub. Zo kan de gebruiker altijd terug naar een werkende versie.
- Houd `README.md` bij: wat de app doet, hoe je hem start, welke gegevens hij gebruikt en wie eigenaar is.
- Vraagt de gebruiker om een kwaliteitstoets? Volg dan de instructies in `prompts/kwaliteitstoets.md`.

## Techniek

- Het project gebruikt React 18 met Vite. In een Codespace start de app automatisch op poort 3000; anders met `npm start`.
- Houd het eenvoudig. Gebruik geen extra bibliotheken als het met React zelf kan.
- Zet herbruikbare onderdelen in aparte componenten in `src/components/`. Houd bestanden klein en overzichtelijk.
- Bewaar gegevens voorlopig in de browser (localStorage), tenzij de gebruiker bewust voor iets anders kiest.
- Schrijf tests (Vitest en Testing Library) voor de belangrijkste functionaliteit.

## Veiligheid en privacy

- Zet nooit API-sleutels, wachtwoorden of tokens in de code. Alles in de frontend is zichtbaar voor iedere bezoeker van de app.
- Gebruik alleen fictieve of geanonimiseerde voorbeeldgegevens.
- Waarschuw de gebruiker als een verzoek persoonsgegevens of vertrouwelijke gegevens raakt.
- Controleer invoer van gebruikers en toon begrijpelijke foutmeldingen.
