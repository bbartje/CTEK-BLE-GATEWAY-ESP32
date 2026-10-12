# Procedure voor bouwen, bewaren en vrijgeven

Eigenaar en beheerder: **bbartje**. Actuele testversie: **0.2.0-beta.9**.

## Wat betekent vrijgegeven?

| Stand | Betekenis |
|---|---|
| Code en bestanden op GitHub | Opgeslagen en terug te vinden in een commit; nog geen bewijs van een geslaagde praktijktest. |
| GitHub-bètarelease (`prerelease`) | Een vaste versietag, beschrijving en gecontroleerde downloads voor bètatests. |
| Eigen praktijktest afgerond | Eigenaar heeft de echte uitslagen vastgelegd, inclusief mislukt en niet getest. |
| Tweede tester uitgenodigd | Tester krijgt dezelfde vaste versie en volgt zelfstandig de startgids op eigen hardware. |
| Stabiele versie 1.0.0 | Eigen en externe installatie-, meet-, herstel- en updatetests zijn geslaagd en de eigenaar heeft dit besloten. |

Beta.9 wordt als bètarelease gepubliceerd. De eigen praktijktest loopt;
een publicatie vult geen hardwaretestvakjes in.

## Voor een firmwarewijziging

1. Verhoog `kFirmwareVersion` in `firmware/include/gateway_types.h` voor een
   volgende functionele versie. Bewaar de eerder gedeelde binaries.
2. Werk README, `firmware/README.md`, de releasenotities, startgids,
   installatiehandleiding, acceptatiechecklist, testtemplate en de
   versie in `.github/workflows/firmware.yml` bij.
3. Bouw en controleer het pakket:

   ```sh
   python3 firmware/tools/build_release.py
   npm run build
   npm test
   git diff --check
   ```

   PlatformIO Core 6.1.19 is nodig. Eventueel gebruik je
   `--pio /pad/naar/pio`. De broncontrole en checksumcontrole horen bij
   de werkelijk geleverde releasebestanden. Licentieteksten van derden
   worden ongewijzigd overgenomen, inclusief hun oorspronkelijke opmaak.
4. Controleer dat de ZIP de factory- en OTA-bin, `manifest.json`,
   `SHA256SUMS.txt`, alle testerinstructies en de bibliotheeklicenties bevat.
   Geen privégegevens, codes of wachtwoorden toevoegen.
5. Commit met een duidelijk Nederlands bericht en push naar GitHub.
   Controleer de build in **Actions** voor die commit.

## Voor alleen documentatie bij dezelfde firmware

Werk de teksten en de kopieën in het pakket bij, bouw het pakket opnieuw en
voer dezelfde controles uit. Vergelijk de factory- en OTA-bin met de al
gedeelde bestanden: die moeten byte voor byte gelijk blijven. Als runtimecode
of binaries veranderen, gebruik dan een volgende bètaversie.

Werk een gepubliceerde vaste release niet stilzwijgend bij. Leg een correctie
zichtbaar vast en gebruik zo nodig een nieuw documentatiepakket of een nieuwe
bètaversie. De beta.9-release omvat de documentatie zoals gecontroleerd vóór
het plaatsen van de versietag.

## GitHub-bètarelease publiceren

1. Gebruik de geteste commit als doel van `v<versie>`, bijvoorbeeld
   `v0.2.0-beta.9`. Laat de tag na publicatie op die commit staan.
2. Gebruik de bijbehorende `docs/RELEASE_<versie>.md` als beschrijving.
   Leg softwarecontroles, open hardwaretests en installatiekeuze vast.
3. Voeg de gecontroleerde OTA-bin, factory-bin, complete ZIP,
   `SHA256SUMS.txt` en `manifest.json` toe. Markeer de release als
   **prerelease**. Gebruik de stabiele downloadlinks van die release.
4. Download de gepubliceerde bestanden opnieuw en vergelijk ze met de lokale
   gecontroleerde bestanden. Controleer ook tag, omschrijving en bètastatus.
5. Meld aan de eigenaar welke commit en release gepubliceerd zijn en welke
   praktijktest nog openstaat.

## Tweede tester starten en resultaten bewaren

De eigenaar rondt eerst de eigen [acceptatiechecklist](ACCEPTATIETEST.md) af
en meldt de uitslag. Bij mislukkingen eerst herstellen of expliciet als
ongetest scenario laten onderzoeken binnen de beperkte bèta.

Stuur daarna zelf de vaste releaselink en [TESTER_START.md](TESTER_START.md)
naar de tweede tester. Geef [TESTVERSLAG_TEMPLATE.md](TESTVERSLAG_TEMPLATE.md)
mee. Bewaar alleen geanonimiseerde uitslagen in de repository en werk de
actuele stand bij in `docs/VERVOLGTEST.md`. Neem geen wachtwoorden of codes
van testers over. Totdat hun uitslag is ontvangen blijft de externe test
**niet getest**.
