#include "gateway_types.h"
#include "admin_access.h"
#include "firmware_image.h"
#include "time_zone.h"
#include "web_ui.h"
#include <ArduinoJson.h>
#include <Update.h>
#include <ctime>
#include <WebServer.h>
#include <WiFi.h>
#include <esp_system.h>
#include <cmath>
#include <cstdlib>

namespace {
WebServer web(80);
String csrf, otaError;
bool uploadAllowed = false, uploadStarted = false, uploadDone = false;
uint8_t otaPrefix[ctek::kImagePrefixSize]{};
size_t prefixSize = 0, uploadedBytes = 0;
uint32_t otaActivity = 0;
String escape(const String& text);
String localized(const char* dutch, const char* english, const char* german) {
  return String("<span data-nl='") + escape(dutch) + "' data-en='" + escape(english) +
         "' data-de='" + escape(german) + "'>" + escape(dutch) + "</span>";
}
String localTime(uint64_t epoch) {
  if (!epoch) return "";
  time_t value = static_cast<time_t>(epoch);
  struct tm* parts = localtime(&value);
  if (!parts) return "";
  char formatted[24]{};
  if (!strftime(formatted, sizeof(formatted), "%Y-%m-%d %H:%M:%S", parts)) return "";
  return formatted;
}
bool authenticated() {
  return web.header("Authorization").startsWith("Basic ") &&
      web.authenticate("admin", config.adminPassword.c_str());
}
bool recoveryAuthenticated() {
  // Check both socket addresses: LAN clients must not use the public credential,
  // even if their router has a route to the setup IP.
  const auto client = web.client();
  const uint32_t mask = uint32_t(WiFi.softAPSubnetMask());
  bool setupPeer = mask && (uint32_t(client.remoteIP()) & mask) == (uint32_t(WiFi.softAPIP()) & mask);
  return web.header("Authorization").startsWith("Basic ") &&
      ctek::recoveryLoginAllowed(initialSetupPending(), recoveryAccessOpen(),
                                   client.localIP() == WiFi.softAPIP(), setupPeer) &&
      web.authenticate("admin", ctek::kInitialAdminPassword);
}
String initialPasswordMessage(const char* message) {
  if (!message) return "";
  if (!strcmp(message, "Gebruik 12–63 zichtbare tekens zonder spaties; letters, cijfers en gewone leestekens zijn toegestaan."))
    return localized("Gebruik 12–63 zichtbare tekens zonder spaties; letters, cijfers en gewone leestekens zijn toegestaan.", "Use 12–63 visible characters without spaces; letters, numbers, and ordinary punctuation are allowed.", "Verwenden Sie 12–63 sichtbare Zeichen ohne Leerzeichen; Buchstaben, Zahlen und übliche Satzzeichen sind erlaubt.");
  if (!strcmp(message, "Kies een eigen wachtwoord dat anders is dan het startwachtwoord."))
    return localized("Kies een eigen wachtwoord dat anders is dan het startwachtwoord.", "Choose a personal password different from the initial password.", "Wählen Sie ein eigenes Passwort, das sich vom Startpasswort unterscheidet.");
  if (!strcmp(message, "De twee wachtwoorden zijn niet gelijk. Vul ze opnieuw in."))
    return localized("De twee wachtwoorden zijn niet gelijk. Vul ze opnieuw in.", "The passwords do not match. Enter them again.", "Die Passwörter stimmen nicht überein. Geben Sie sie erneut ein.");
  return localized("Het wachtwoord kon niet worden opgeslagen. De eerste instelling blijft geblokkeerd; probeer opnieuw.", "The password could not be saved. Initial setup remains locked; try again.", "Das Passwort konnte nicht gespeichert werden. Die Ersteinrichtung bleibt gesperrt; versuchen Sie es erneut.");
}

String escape(const String& text) {
  String output;
  for (char c : text) {
    if (c == '&') output += "&amp;";
    else if (c == '<') output += "&lt;";
    else if (c == '>') output += "&gt;";
    else if (c == '"') output += "&quot;";
    else if (c == '\'') output += "&#39;";
    else output += c;
  }
  return output;
}
bool authorized(bool passwordPage = false) {
  web.sendHeader("Cache-Control", "no-store");
  auto access = ctek::webAccess(authenticated(), initialSetupPending(), passwordPage,
                              recoveryAuthenticated());
  if (access == ctek::WebAccess::Allowed) return true;
  if (access == ctek::WebAccess::PasswordChangeRequired) {
    web.sendHeader("Location", "/password"); web.send(303); return false;
  }
  web.requestAuthentication(BASIC_AUTH, "CTEK gateway"); return false;
}
bool mutation(bool passwordPage = false) {
  if (!authorized(passwordPage)) return false;
  if (web.arg("csrf") == csrf) return true;
  web.send(403, "text/plain; charset=utf-8", "Formulier verlopen. Open de hoofdpagina opnieuw.");
  return false;
}
String startPage() {
  return FPSTR(ctek::kPageStart);
}
String token() { return "<input type='hidden' name='csrf' value='" + csrf + "'>"; }
String input(const char* name, const char* dutch, const char* english, const char* german,
             const String& value, const char* options = "") {
  return String("<label>") + localized(dutch, english, german) + "<input name='" + name +
         "' value='" + escape(value) + "' " + options + "></label>";
}
void redirect() { web.sendHeader("Location", "/"); web.send(303); }
void error(const char* message) { web.send(400, "text/plain; charset=utf-8", message); }
void passwordPage(const char* message = nullptr) {
  String page = startPage();
  if (initialSetupPending()) page += "<h2>" + localized("Kies eerst je eigen wachtwoord", "Choose your own password first", "Legen Sie zuerst Ihr eigenes Passwort fest") + "</h2><p>" +
          localized("Welkom. Vervang het eenmalige startwachtwoord voordat je de gateway instelt. Je eigen wachtwoord geldt voor de beheerlogin en het setupnetwerk.",
                    "Welcome. Replace the one-time initial password before configuring the gateway. Your password is used for both the admin login and setup network.",
                    "Willkommen. Ersetzen Sie das einmalige Startpasswort, bevor Sie das Gateway konfigurieren. Ihr Passwort gilt für die Admin-Anmeldung und das Setup-Netzwerk.") + "</p>";
  else page += "<h2>" + localized("Beheerwachtwoord herstellen", "Recover admin password", "Admin-Passwort zurücksetzen") + "</h2><p>" +
      localized("Via deze herstel-wifi kun je alleen een nieuw beheerwachtwoord kiezen. Je wifi-, MQTT- en sensorinstellingen blijven bewaard. Sla op binnen twee minuten na de herstart.",
                "This recovery Wi-Fi only allows choosing a new admin password. Wi-Fi, MQTT, and sensor settings are preserved. Save within two minutes of restarting.",
                "Über dieses Wiederherstellungs-WLAN können Sie nur ein neues Admin-Passwort festlegen. WLAN-, MQTT- und Sensoreinstellungen bleiben erhalten. Speichern Sie innerhalb von zwei Minuten nach dem Neustart.") + "</p>";
  if (message) page += "<p role='alert'>" + initialPasswordMessage(message) + "</p>";
  page += "<form method='post' action='/password'>" + token();
  page += input("new_password", "Nieuw wachtwoord (12–63 tekens, zonder spaties)", "New password (12–63 characters, no spaces)", "Neues Passwort (12–63 Zeichen, keine Leerzeichen)", "", "type='password' required minlength='12' maxlength='63' autocomplete='new-password'");
  page += input("confirm_password", "Herhaal het nieuwe wachtwoord", "Confirm the new password", "Neues Passwort wiederholen", "", "type='password' required minlength='12' maxlength='63' autocomplete='new-password'");
  page += "<button>" + localized("Wachtwoord opslaan en herstarten", "Save password and restart", "Passwort speichern und neu starten") + "</button></form><p>";
  page += initialSetupPending() ? localized("De gateway herstart na het opslaan. Verbind daarna opnieuw met het setupnetwerk met je nieuwe wachtwoord en open http://192.168.4.1. Verwijder zo nodig het oude wifi-netwerk uit je opgeslagen netwerken.",
                    "The gateway restarts after saving. Reconnect to the setup network with your new password and open http://192.168.4.1. If needed, forget the old Wi-Fi network on your device.",
                    "Das Gateway startet nach dem Speichern neu. Verbinden Sie sich erneut mit dem Setup-Netzwerk und Ihrem neuen Passwort und öffnen Sie http://192.168.4.1. Löschen Sie bei Bedarf das alte WLAN auf Ihrem Gerät.") :
      localized("Na de herstart open je de gateway via je gewone netwerk. Log in als admin met het nieuwe wachtwoord.",
                "After restarting, open the gateway on your normal network. Sign in as admin with the new password.",
                "Öffnen Sie das Gateway nach dem Neustart über Ihr normales Netzwerk. Melden Sie sich als admin mit dem neuen Passwort an.");
  page += "</p></body></html>";
  web.send(message ? 400 : 200, "text/html; charset=utf-8", page);
}
void firstPassword() {
  if (!authorized(true)) return;
  if (!initialSetupPending() && !recoveryAuthenticated()) { redirect(); return; }
  passwordPage();
}
void setFirstPassword() {
  if (!mutation(true)) return;
  if (!initialSetupPending() && !recoveryAuthenticated()) { redirect(); return; }
  bool recovering = !initialSetupPending();
  String password = web.arg("new_password"), confirmation = web.arg("confirm_password");
  auto problem = ctek::checkInitialPassword(std::string_view(password.c_str(), password.length()),
                                          std::string_view(confirmation.c_str(), confirmation.length()));
  if (problem == ctek::PasswordProblem::Invalid) {
    passwordPage("Gebruik 12–63 zichtbare tekens zonder spaties; letters, cijfers en gewone leestekens zijn toegestaan."); return;
  }
  if (problem == ctek::PasswordProblem::UnchangedDefault) {
    passwordPage("Kies een eigen wachtwoord dat anders is dan het startwachtwoord."); return;
  }
  if (problem == ctek::PasswordProblem::Mismatch) {
    passwordPage("De twee wachtwoorden zijn niet gelijk. Vul ze opnieuw in."); return;
  }
  if (!saveAdminPassword(password)) {
    passwordPage("Het wachtwoord kon niet worden opgeslagen. De eerste instelling blijft geblokkeerd; probeer opnieuw."); return;
  }
  String page = startPage() + "<h2>" + localized("Eigen wachtwoord opgeslagen", "Password saved", "Passwort gespeichert") + "</h2><p>";
  page += recovering ? localized("De gateway herstart nu. Verbind met je gewone netwerk, open de gateway en log in als admin met je nieuwe wachtwoord. Alle andere instellingen blijven bewaard.",
                                 "The gateway is restarting. Connect to your normal network, open the gateway, and sign in as admin with your new password. All other settings are preserved.",
                                 "Das Gateway startet neu. Verbinden Sie sich mit Ihrem normalen Netzwerk, öffnen Sie das Gateway und melden Sie sich als admin mit Ihrem neuen Passwort an. Alle anderen Einstellungen bleiben erhalten.") :
      localized("De gateway herstart nu. Verbind opnieuw met het setupnetwerk met je nieuwe wachtwoord en open http://192.168.4.1. Vergeet zo nodig het oude wifi-netwerk. Log in als admin met je nieuwe wachtwoord om wifi, MQTT en sensoren in te stellen.",
                "The gateway is restarting. Reconnect to the setup network with your new password and open http://192.168.4.1. Forget the old Wi-Fi network if needed. Sign in as admin with your new password to configure Wi-Fi, MQTT, and sensors.",
                "Das Gateway startet neu. Verbinden Sie sich erneut mit dem Setup-Netzwerk und Ihrem neuen Passwort und öffnen Sie http://192.168.4.1. Löschen Sie bei Bedarf das alte WLAN. Melden Sie sich als admin mit Ihrem neuen Passwort an, um WLAN, MQTT und Sensoren einzurichten.");
  web.send(200, "text/html; charset=utf-8", page + "</p></body></html>");
  delay(400); ESP.restart();
}
bool number(const String& text, long minimum, long maximum, long& output) {
  if (text.isEmpty() || text.length() > 10) return false;
  for (char c : text) if (c < '0' || c > '9') return false;
  char* end = nullptr; output = strtol(text.c_str(), &end, 10);
  return end && !*end && output >= minimum && output <= maximum;
}
bool slotArgument(size_t& slot) {
  long value;
  if (!number(web.arg("slot"), 0, ctek::kMaxSensors - 1, value)) return false;
  slot = value; return true;
}

void root() {
  if (!authorized()) return;
  String page = startPage();
  page += "<p>" + localized("Firmware ", "Firmware ", "Firmware ") + String(kFirmwareVersion) + localized(" | wifi: ", " | Wi-Fi: ", " | WLAN: ");
  page += WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : localized("setupmodus / niet verbonden", "setup mode / disconnected", "Setup-Modus / nicht verbunden");
  page += localized(" | MQTT: ", " | MQTT: ", " | MQTT: ") + (mqttConnected() ? localized("verbonden", "connected", "verbunden") : localized("niet verbonden", "disconnected", "nicht verbunden")) +
          " (" + localized("status", "status", "Status") + " " + String(mqttStatus()) + ")</p><p>" + localized("Vrij geheugen: ", "Free memory: ", "Freier Speicher: ") + String(ESP.getFreeHeap()) +
          localized(" bytes; minimum: ", " bytes; minimum: ", " Bytes; Minimum: ") + String(ESP.getMinFreeHeap()) + localized(" bytes. Tijd: ", " bytes. Time: ", " Bytes. Zeit: ") +
          (validEpoch() ? localized("gesynchroniseerd", "synchronized", "synchronisiert") : localized("nog niet gesynchroniseerd", "not synchronized", "nicht synchronisiert")) + "</p>";
  page += "<form method='post' action='/save'>" + token() + "<fieldset><legend>" + localized("Wifi en MQTT", "Wi-Fi and MQTT", "WLAN und MQTT") + "</legend>";
  page += input("ssid", "Wifi-netwerk (2,4 GHz)", "Wi-Fi network (2.4 GHz)", "WLAN-Netzwerk (2,4 GHz)", config.ssid, "required maxlength='32'");
  page += input("wifi_password", "Wifi-wachtwoord; leeg = behouden", "Wi-Fi password; leave blank to keep", "WLAN-Passwort; leer lassen zum Beibehalten", "", "type='password' maxlength='63' autocomplete='new-password'");
  page += "<label><input style='width:auto' name='open_wifi' type='checkbox' value='1'>" + localized("Dit netwerk heeft geen wachtwoord", "This network has no password", "Dieses Netzwerk hat kein Passwort") + "</label>";
  page += input("mqtt_host", "MQTT-brokeradres (IP of hostnaam)", "MQTT broker address (IP or hostname)", "MQTT-Brokeradresse (IP oder Hostname)", config.mqttHost, "maxlength='128'");
  page += input("mqtt_port", "MQTT-poort", "MQTT port", "MQTT-Port", String(config.mqttPort), "type='number' min='1' max='65535' required");
  page += input("mqtt_user", "MQTT-gebruikersnaam", "MQTT username", "MQTT-Benutzername", config.mqttUser, "maxlength='64'");
  page += input("mqtt_password", "MQTT-wachtwoord; leeg = behouden", "MQTT password; leave blank to keep", "MQTT-Passwort; leer lassen zum Beibehalten", "", "type='password' maxlength='128' autocomplete='new-password'");
  page += "<label><span>" + localized("Tijdzone", "Time zone", "Zeitzone") + "</span><select name='timezone' required>";
  for (const auto& zone : ctek::kTimeZones) {
    String zoneName = zone.name;
    page += "<option value='" + escape(zoneName) + "'" + String(config.timezone == zone.name ? " selected" : "") + ">" + escape(zoneName) + "</option>";
  }
  page += "</select></label>";
  page += input("poll_seconds", "Uitleesinterval per sensor (seconden, 60–900)", "Polling interval per sensor (seconds, 60–900)", "Abfrageintervall je Sensor (Sekunden, 60–900)", String(config.pollSeconds), "type='number' min='60' max='900' required");
  page += input("low_voltage", "Waarschuw onder deze spanning (volt)", "Low-voltage warning threshold (volts)", "Schwellwert für Unterspannung (Volt)", String(config.lowVoltage, 2), "type='number' min='8' max='16' step='0.01' required");
  page += input("admin_password", "Nieuw beheer-/setupwachtwoord; leeg = behouden (12–63 tekens)", "New admin/setup password; leave blank to keep (12–63 characters)", "Neues Admin-/Setup-Passwort; leer lassen zum Beibehalten (12–63 Zeichen)", "", "type='password' minlength='12' maxlength='63' autocomplete='new-password'");
  page += "</fieldset><button>" + localized("Instellingen opslaan en herstarten", "Save settings and restart", "Einstellungen speichern und neu starten") + "</button></form>";
  page += "<h2>" + localized("Batterijmonitors", "Battery monitors", "Batteriemonitore") + "</h2><p>" + localized("Verbindingen gaan om de beurt. Oude metingen vervallen na 30 minuten.", "Sensors are read one at a time. Readings expire after 30 minutes.", "Die Sensoren werden nacheinander ausgelesen. Messwerte verfallen nach 30 Minuten.") + "</p>";
  page += "<form method='post' action='/scan'>" + token() + "<button>" + localized("Zoek Bluetooth-apparaten (6 seconden)", "Scan for Bluetooth devices (6 seconds)", "Nach Bluetooth-Geräten suchen (6 Sekunden)") + "</button></form>";
  if (scanRequested || (workInFlight && !scanFinished)) page += "<p>" + localized("Zoeken of uitlezen bezig. Vernieuw deze pagina na enkele seconden.", "Scan or read in progress. Refresh this page in a few seconds.", "Suche oder Abfrage läuft. Aktualisieren Sie diese Seite in einigen Sekunden.") + "</p>";
  if (scanFinished) {
    page += "<details open><summary>" + localized("Scanresultaten", "Scan results", "Suchergebnisse") + "</summary><ul>";
    for (size_t i = 0; i < scanCount; ++i) {
      const auto& found = scanEntries[i];
      page += "<li>" + escape(found.name) + " " + escape(found.address) + " | " + localized("type", "type", "Typ") + " " + String(found.addressType) +
              " | " + String(found.rssi) + " dBm | " + (found.connectable ? localized("verbindbaar", "connectable", "verbindbar") : localized("niet verbindbaar", "not connectable", "nicht verbindbar")) +
              (found.candidate ? " | " + localized("CTEK-kandidaat", "CTEK candidate", "CTEK-Kandidat") : "") + "</li>";
    }
    if (!scanCount) page += "<li>" + localized("Geen apparaten gezien. Controleer bereik en sluit de CTEK-app.", "No devices found. Check the range and close the CTEK app.", "Keine Geräte gefunden. Prüfen Sie die Reichweite und schließen Sie die CTEK-App.") + "</li>";
    page += "</ul><small>" + localized("Kopieer het adres en het adrestype naar een vrije plek hieronder. De GATT-service bevestigt CTEK.", "Copy the address and address type into an available slot below. The GATT service confirms CTEK.", "Kopieren Sie Adresse und Adresstyp in einen freien Platz unten. Der GATT-Dienst bestätigt CTEK.") + "</small></details>";
  }
  for (size_t slot = 0; slot < ctek::kMaxSensors; ++slot) {
    const auto& state = states[slot]; const auto& sensor = sensors[slot];
    page += "<article class='sensor-slot' data-configured='" + String(state.configured ? "1" : "0") + "'" + (state.configured ? "" : " hidden") + "><h3>" + localized("Plek ", "Slot ", "Platz ") + String(slot + 1) + " — " + escape(state.configured ? sensor.name : String("vrij")) + "</h3>";
    if (state.configured) {
      if (state.status == "Meting ontvangen") page += "<p>" + localized("Meting ontvangen", "Reading received", "Messwert empfangen") + "</p>";
      else if (state.status == "Bezig met BLE-uitlezing") page += "<p>" + localized("Bezig met BLE-uitlezing", "BLE read in progress", "BLE-Abfrage läuft") + "</p>";
      else page += "<p>" + escape(state.status) + "</p>";
      if (state.hasReading) {
        const auto& sample = state.reading.samples[state.reading.count - 1];
        page += "<p>";
        if (state.reading.currentVoltageValid) page += localized("Spanning nu: ", "Live voltage: ", "Aktuelle Spannung: ") + String(state.reading.currentVoltage, 3) + " V; ";
        else page += localized("Live-spanning niet beschikbaar; laatste historische spanning: ", "Live voltage unavailable; last historical voltage: ", "Aktuelle Spannung nicht verfügbar; letzte historische Spannung: ") + String(sample.voltage, 3) + " V; ";
        page += readingFresh(slot) ? localized("actueel", "current", "aktuell") : localized("verouderd", "stale", "veraltet");
        if (state.reading.currentTemperatureValid) page += "; " + localized("Temperatuur nu: ", "Current temperature: ", "Aktuelle Temperatur: ") + String(state.reading.currentTemperature, 1) + " °C";
        else if (!state.reading.live) page += "; " + localized("Historische temperatuur: ", "Historical temperature: ", "Historische Temperatur: ") + String(sample.temperature, 1) + " °C";
        page += "</p>";
        page += "<p>" + localized("Laatste geslaagde uitlezing: ", "Last successful read: ", "Letzte erfolgreiche Abfrage: ");
        page += state.readEpoch ? localTime(state.readEpoch) : localized("tijd niet gesynchroniseerd", "time not synchronized", "Zeit nicht synchronisiert");
        page += "</p>";
      }
      page += "<small>" + localized("Pogingen ", "Attempts ", "Versuche ") + String(state.attempts) + "; " + localized("geslaagd ", "successful ", "erfolgreich ") + String(state.successes) + "; " + localized("opeenvolgende fouten ", "consecutive failures ", "aufeinanderfolgende Fehler ") + String(state.failures) + "</small>";
      page += "<form method='post' action='/poll'>" + token() + "<input type='hidden' name='slot' value='" + String(slot) + "'><button>" + localized("Nu uitlezen", "Read now", "Jetzt abfragen") + "</button></form>";
      page += "<details><summary>" + localized("BLE-log", "BLE log", "BLE-Protokoll") + "</summary><pre>" + escape(state.log) + "</pre></details>";
    }
    page += "<form method='post' action='/sensor'>" + token() + "<input type='hidden' name='slot' value='" + String(slot) + "'>";
    page += input("name", "Naam", "Name", "Name", sensor.name, "maxlength='40'");
    page += input("address", "Bluetooth-adres, bijvoorbeeld 2C:FC:E4:01:02:03", "Bluetooth address, for example 2C:FC:E4:01:02:03", "Bluetooth-Adresse, zum Beispiel 2C:FC:E4:01:02:03", sensor.address, "required minlength='17' maxlength='17'");
    page += input("address_type", "Adrestype uit de scan (meestal 0)", "Address type from scan (usually 0)", "Adresstyp aus dem Scan (meist 0)", String(sensor.addressType), "type='number' min='0' max='3' required");
    page += input("code", state.configured ? "Verbindingscode; leeg = behouden bij hetzelfde adres" : "Verbindingscode: 16 tekens, of 11 tekens zonder 40149", state.configured ? "Pairing code; leave blank to keep for the same address" : "Pairing code: 16 characters, or 11 without 40149", state.configured ? "Verbindungscode; leer lassen für dieselbe Adresse" : "Verbindungscode: 16 Zeichen oder 11 ohne 40149", "", "type='password' maxlength='64' autocomplete='new-password'");
    page += "<button>" + localized("Sensor opslaan", "Save sensor", "Sensor speichern") + "</button></form>";
    if (state.configured) page += "<form method='post' action='/remove'>" + token() + "<input type='hidden' name='slot' value='" + String(slot) + "'><button>" + localized("Sensor verwijderen", "Remove sensor", "Sensor entfernen") + "</button></form>";
    page += "</article>";
  }
  page += "<button id='add-sensor' type='button' aria-label='Add sensor'>+</button>";
  page += "<h2>" + localized("Onderhoud via wifi", "Maintenance over Wi-Fi", "Wartung über WLAN") + "</h2><a class='button' href='/diagnostics'>" + localized("Diagnostics downloaden", "Download diagnostics", "Diagnose herunterladen") + "</a> ";
  page += "<form method='post' action='/ota-mode'>" + token() + "<button>" + localized("Uitlezen pauzeren en firmware bijwerken", "Pause polling and update firmware", "Abfragen pausieren und Firmware aktualisieren") + "</button></form>";
  page += "<small>" + localized("Gegevens blijven op het bord en gaan naar je eigen MQTT-broker. Gebruik deze HTTP-/MQTT-verbindingen op je eigen vertrouwde netwerk. De CTEK-app kan een sensor bezet houden.", "Data stays on the board and is sent to your own MQTT broker. Use these HTTP/MQTT connections on a trusted network. The CTEK app may keep a sensor busy.", "Daten bleiben auf dem Gerät und werden an Ihren MQTT-Broker gesendet. Verwenden Sie HTTP/MQTT nur in einem vertrauenswürdigen Netzwerk. Die CTEK-App kann einen Sensor belegen.") + "</small></body></html>";
  web.sendHeader("Cache-Control", "no-store");
  web.send(200, "text/html; charset=utf-8", page);
}

void save() {
  if (!mutation()) return;
  long port, poll;
  String ssid = web.arg("ssid"), wifi = web.arg("wifi_password"), host = web.arg("mqtt_host");
  String user = web.arg("mqtt_user"), password = web.arg("mqtt_password"), admin = web.arg("admin_password");
  String timezone = web.arg("timezone");
  char* end = nullptr; String voltageText = web.arg("low_voltage");
  float voltage = strtof(voltageText.c_str(), &end);
  if (ssid.isEmpty() || ssid.length() > 32 || wifi.length() > 63 || host.length() > 128 ||
      user.length() > 64 || password.length() > 128 ||
      !number(web.arg("mqtt_port"), 1, 65535, port) || !number(web.arg("poll_seconds"), 60, 900, poll) ||
      !ctek::findTimeZone(timezone.c_str()) ||
      voltageText.isEmpty() || !end || *end || !std::isfinite(voltage) || voltage < 8 || voltage > 16 ||
      (!admin.isEmpty() && !ctek::validReplacementPassword(std::string_view(admin.c_str(), admin.length())))) {
    error("Ongeldige instellingen. Controleer netwerknamen, poort, tijdzone, interval, spanning en wachtwoordlengte."); return;
  }
  for (char c : host) if (!(isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == ':')) {
    error("Gebruik alleen een IP-adres of hostnaam voor de MQTT-broker, zonder http:// of spaties."); return;
  }
  if (web.arg("open_wifi") != "1" && (wifi.isEmpty() ? config.wifiPassword.length() : wifi.length()) < 8) {
    error("Vul een wifi-wachtwoord van 8–63 tekens in, of kies bewust een netwerk zonder wachtwoord."); return;
  }
  if (!admin.isEmpty() && !saveAdminPassword(admin)) {
    error("Het nieuwe beheerwachtwoord kon niet worden opgeslagen. Probeer opnieuw."); return;
  }
  config.ssid = ssid; config.mqttHost = host; config.mqttPort = port; config.mqttUser = user;
  config.pollSeconds = poll; config.lowVoltage = voltage; config.timezone = timezone;
  if (web.arg("open_wifi") == "1") config.wifiPassword = "";
  else if (!wifi.isEmpty()) config.wifiPassword = wifi;
  if (!password.isEmpty()) config.mqttPassword = password;
  saveConfiguration();
  web.send(200, "text/html; charset=utf-8", startPage() + "<p>Opgeslagen. De gateway herstart; verbind daarna met het ingestelde netwerk.</p></body></html>");
  delay(400); ESP.restart();
}

void sensor() {
  if (!mutation()) return;
  size_t slot; long type;
  std::string address, code;
  String name = web.arg("name");
  if (!slotArgument(slot) || !ctek::normalizeAddress(web.arg("address").c_str(), address) ||
      !number(web.arg("address_type"), 0, 3, type) || name.length() > 40) {
    error("Ongeldig adres, naam (maximaal 40 tekens), adrestype of sensorplek."); return;
  }
  bool unchangedAddress = states[slot].configured && sensors[slot].address == address.c_str();
  if (web.arg("code").isEmpty() && unchangedAddress) code = sensors[slot].code.c_str();
  else if (!ctek::normalizeCode(web.arg("code").c_str(), code)) {
    error("Verbindingscode moet 16 ASCII-tekens bevatten, of 11 tekens zonder prefix 40149."); return;
  }
  for (size_t i = 0; i < ctek::kMaxSensors; ++i) if (i != slot && states[i].configured && sensors[i].address == address.c_str()) {
    error("Dit Bluetooth-adres is al aan een andere plek gekoppeld."); return;
  }
  if (states[slot].configured && !unchangedAddress) removeDiscovery(slot);
  uint32_t generation = states[slot].generation + 1;
  states[slot] = SensorState{}; states[slot].generation = generation; states[slot].configured = true;
  sensors[slot].address = address.c_str(); sensors[slot].code = code.c_str(); sensors[slot].addressType = type;
  sensors[slot].name = name.isEmpty() ? String("CTEK ") + sensors[slot].address.substring(12) : name;
  saveSensors(); redirect();
}
void removeSensor() {
  if (!mutation()) return;
  size_t slot; if (!slotArgument(slot)) { error("Ongeldige sensorplek."); return; }
  removeDiscovery(slot);
  uint32_t generation = states[slot].generation + 1;
  states[slot] = SensorState{}; states[slot].generation = generation;
  sensors[slot] = SensorConfig{};
  saveSensors(); redirect();
}
void poll() {
  if (!mutation()) return;
  size_t slot; if (!slotArgument(slot) || !states[slot].configured) { error("Ongeldige sensorplek."); return; }
  states[slot].retryMs = 0; redirect();
}
void scan() {
  if (!mutation()) return;
  scanRequested = true; scanFinished = false; redirect();
}
void diagnostics() {
  if (!authorized()) return;
  JsonDocument doc;
  doc["firmware"] = kFirmwareVersion; doc["chip"] = "ESP32";
  doc["flash_bytes"] = ESP.getFlashChipSize(); doc["free_heap"] = ESP.getFreeHeap();
  doc["min_free_heap"] = ESP.getMinFreeHeap(); doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
  doc["mqtt_connected"] = mqttConnected(); doc["mqtt_status"] = mqttStatus();
  doc["time_synchronized"] = validEpoch() != 0; doc["uptime_seconds"] = millis() / 1000;
  auto list = doc["sensors"].to<JsonArray>();
  for (size_t i = 0; i < ctek::kMaxSensors; ++i) {
    const auto& state = states[i]; if (!state.configured) continue;
    auto item = list.add<JsonObject>();
    item["slot"] = i + 1; item["address"] = sensors[i].address; item["address_type"] = sensors[i].addressType;
    item["status"] = state.status; item["attempts"] = state.attempts; item["successes"] = state.successes;
    item["fresh"] = readingFresh(i); item["log"] = state.log;
    if (state.hasReading) { item["sample_age_seconds"] = readingAge(i); item["cursor"] = state.reading.metadata.cursor; }
  }
  String payload; serializeJsonPretty(doc, payload);
  web.sendHeader("Content-Disposition", "attachment; filename=ctek-diagnostics.json");
  web.sendHeader("Cache-Control", "no-store");
  web.send(200, "application/json", payload);
}

void updatePage() {
  if (!authorized()) return;
  if (!otaMode) { web.send(409, "text/plain; charset=utf-8", "Activeer eerst de updatepauze op de hoofdpagina."); return; }
  otaActivity = millis();
  String page = startPage();
  if (workInFlight) page += "<p>" + localized("De huidige Bluetooth-poging wordt afgerond. Vernieuw deze pagina over enkele seconden.", "The current Bluetooth read is finishing. Refresh this page in a few seconds.", "Die aktuelle Bluetooth-Abfrage wird beendet. Aktualisieren Sie diese Seite in einigen Sekunden.") + "</p>";
  else page += "<p>" + localized("Uitlezen gepauzeerd. Upload alleen", "Polling is paused. Upload only", "Abfragen pausiert. Laden Sie nur") + " <strong>ctek-esp32-wroom-32-ota.bin</strong>. " + localized("De factory-bin is voor USB.", "The factory image is for USB installation.", "Das Factory-Image ist für die USB-Installation.") + "</p><form method='post' action='/update?csrf=" + csrf + "' enctype='multipart/form-data'><input name='firmware' type='file' accept='.bin' required><button>" + localized("Firmware uploaden", "Upload firmware", "Firmware hochladen") + "</button></form>";
  page += "<form method='post' action='/resume'>" + token() + "<button>" + localized("Annuleren en uitlezen hervatten", "Cancel and resume polling", "Abbrechen und Abfragen fortsetzen") + "</button></form></body></html>";
  web.send(200, "text/html; charset=utf-8", page);
}
void failUpload(const String& reason) { otaError = reason; if (uploadStarted) Update.abort(); uploadStarted = false; }
void upload() {
  auto& current = web.upload(); otaActivity = millis();
  if (current.status == UPLOAD_FILE_START) {
    uploadAllowed = ctek::webAccess(authenticated(), initialSetupPending()) == ctek::WebAccess::Allowed &&
                    web.arg("csrf") == csrf && otaMode && !workInFlight;
    uploadStarted = uploadDone = false; otaError = ""; prefixSize = uploadedBytes = 0;
    if (!uploadAllowed) otaError = "Update niet toegestaan: controleer login en updatepauze.";
  } else if (current.status == UPLOAD_FILE_WRITE && uploadAllowed && otaError.isEmpty()) {
    if (uploadedBytes + current.currentSize > ctek::kAppSlotSize) { failUpload("Firmware te groot voor deze 4 MB-partitie."); return; }
    size_t offset = 0;
    while (prefixSize < sizeof(otaPrefix) && offset < current.currentSize) otaPrefix[prefixSize++] = current.buf[offset++];
    if (!uploadStarted && prefixSize == sizeof(otaPrefix)) {
      if (!ctek::validOtaPrefix(otaPrefix, prefixSize)) { failUpload("Verkeerd bestand: verwacht CTEK-app voor ESP32, geen factory-bin of andere chip."); return; }
      if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) { failUpload("Vrije updatepartitie niet beschikbaar."); return; }
      uploadStarted = true;
      if (Update.write(otaPrefix, sizeof(otaPrefix)) != sizeof(otaPrefix)) { failUpload("Firmwareheader schrijven mislukt."); return; }
    }
    if (uploadStarted && offset < current.currentSize &&
        Update.write(current.buf + offset, current.currentSize - offset) != current.currentSize - offset) {
      failUpload("Firmware schrijven mislukt."); return;
    }
    uploadedBytes += current.currentSize;
  } else if (current.status == UPLOAD_FILE_END && uploadAllowed && otaError.isEmpty()) {
    if (!uploadStarted || !Update.end(true)) failUpload("Upload onvolledig of firmwarecontrole mislukt.");
    else { uploadDone = true; uploadStarted = false; }
  } else if (current.status == UPLOAD_FILE_ABORTED) { failUpload("Upload afgebroken; de huidige firmware blijft actief."); }
}
}

void startWeb() {
  char nonce[17]; snprintf(nonce, sizeof(nonce), "%08lx%08lx", (unsigned long)esp_random(), (unsigned long)esp_random()); csrf = nonce;
  web.on("/", HTTP_GET, root); web.on("/save", HTTP_POST, save);
  web.on("/password", HTTP_GET, firstPassword); web.on("/password", HTTP_POST, setFirstPassword);
  web.on("/sensor", HTTP_POST, sensor); web.on("/remove", HTTP_POST, removeSensor);
  web.on("/poll", HTTP_POST, poll); web.on("/scan", HTTP_POST, scan);
  web.on("/diagnostics", HTTP_GET, diagnostics);
  web.on("/ota-mode", HTTP_POST, []() {
    if (!mutation()) return;
    otaMode = true; otaActivity = millis();
    web.sendHeader("Location", "/update"); web.send(303);
  });
  web.on("/resume", HTTP_POST, []() { if (!mutation()) return; otaMode = false; redirect(); });
  web.on("/update", HTTP_GET, updatePage);
  web.on("/update", HTTP_POST, []() {
    if (!authorized()) return;
    web.send(uploadDone ? 200 : 400, "text/plain; charset=utf-8", uploadDone ?
      String("Firmware gecontroleerd en opgeslagen. De gateway herstart.") : otaError);
    if (uploadDone) { delay(400); ESP.restart(); }
    otaMode = false;
  }, upload);
  web.onNotFound([]() { web.send(404, "text/plain; charset=utf-8", "Pagina niet gevonden."); });
  web.begin();
}
void serviceWeb() {
  web.handleClient();
  if (otaMode && !uploadStarted && ctek::elapsed(millis(), otaActivity, 300000)) otaMode = false;
}
