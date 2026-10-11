#include "gateway_types.h"
#include "firmware_image.h"
#include <ArduinoJson.h>
#include <Update.h>
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
bool authorized() {
  if (web.authenticate("admin", config.adminPassword.c_str())) return true;
  web.requestAuthentication(BASIC_AUTH, "CTEK gateway"); return false;
}
bool mutation() {
  if (!authorized()) return false;
  if (web.arg("csrf") == csrf) return true;
  web.send(403, "text/plain; charset=utf-8", "Formulier verlopen. Open de hoofdpagina opnieuw.");
  return false;
}
String startPage() {
  return F("<!doctype html><html lang='nl'><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>CTEK gateway</title><style>body{font:16px system-ui;max-width:850px;margin:2rem auto;padding:0 1rem;background:#f5f7f6;color:#172c29}fieldset,article{border:1px solid #abc1ba;border-radius:8px;padding:1rem;margin:1rem 0}label{display:block;margin:.6rem 0}input,select{box-sizing:border-box;width:100%;padding:.55rem}button,a.button{background:#176459;color:#fff;padding:.6rem .9rem;border:0;border-radius:4px;display:inline-block;margin:.4rem .4rem .4rem 0}pre{white-space:pre-wrap;overflow-wrap:anywhere}small{color:#52635f}</style><body><h1>CTEK Battery Sense gateway</h1>");
}
String token() { return "<input type='hidden' name='csrf' value='" + csrf + "'>"; }
String input(const char* name, const char* label, const String& value, const char* options = "") {
  return String("<label>") + label + "<input name='" + name + "' value='" + escape(value) + "' " + options + "></label>";
}
void redirect() { web.sendHeader("Location", "/"); web.send(303); }
void error(const char* message) { web.send(400, "text/plain; charset=utf-8", message); }
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
  page += "<p>Firmware " + String(kFirmwareVersion) + " | wifi: ";
  page += WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("setupmodus / niet verbonden");
  page += " | MQTT: " + String(mqttConnected() ? "verbonden" : "niet verbonden") +
          " (status " + String(mqttStatus()) + ")</p><p>Vrij geheugen: " + String(ESP.getFreeHeap()) +
          " bytes; minimum: " + String(ESP.getMinFreeHeap()) + " bytes. Tijd: " +
          (validEpoch() ? String("gesynchroniseerd") : String("nog niet gesynchroniseerd")) + "</p>";
  page += "<form method='post' action='/save'>" + token() + "<fieldset><legend>Wifi en MQTT</legend>";
  page += input("ssid", "Wifi-netwerk (2,4 GHz)", config.ssid, "required maxlength='32'");
  page += input("wifi_password", "Wifi-wachtwoord; leeg = behouden", "", "type='password' maxlength='63' autocomplete='new-password'");
  page += "<label><input style='width:auto' name='open_wifi' type='checkbox' value='1'>Dit netwerk heeft geen wachtwoord</label>";
  page += input("mqtt_host", "MQTT-brokeradres (IP of hostnaam)", config.mqttHost, "maxlength='128'");
  page += input("mqtt_port", "MQTT-poort", String(config.mqttPort), "type='number' min='1' max='65535' required");
  page += input("mqtt_user", "MQTT-gebruikersnaam", config.mqttUser, "maxlength='64'");
  page += input("mqtt_password", "MQTT-wachtwoord; leeg = behouden", "", "type='password' maxlength='128' autocomplete='new-password'");
  page += input("poll_seconds", "Uitleesinterval per sensor (seconden, 60–900)", String(config.pollSeconds), "type='number' min='60' max='900' required");
  page += input("low_voltage", "Waarschuw onder deze spanning (volt)", String(config.lowVoltage, 2), "type='number' min='8' max='16' step='0.01' required");
  page += input("admin_password", "Nieuw beheer-/setupwachtwoord; leeg = behouden (12–63 tekens)", "", "type='password' minlength='12' maxlength='63' autocomplete='new-password'");
  page += "</fieldset><button>Instellingen opslaan en herstarten</button></form>";
  page += "<h2>Vier batterijmonitors</h2><p>Verbindingen gaan om de beurt. Oude metingen vervallen na 30 minuten.</p>";
  page += "<form method='post' action='/scan'>" + token() + "<button>Zoek Bluetooth-apparaten (6 seconden)</button></form>";
  if (scanRequested || (workInFlight && !scanFinished)) page += "<p>Zoeken of uitlezen bezig. Vernieuw deze pagina na enkele seconden.</p>";
  if (scanFinished) {
    page += "<details open><summary>Scanresultaten</summary><ul>";
    for (size_t i = 0; i < scanCount; ++i) {
      const auto& found = scanEntries[i];
      page += "<li>" + escape(found.name) + " " + escape(found.address) + " | type " + String(found.addressType) +
              " | " + String(found.rssi) + " dBm | " + (found.connectable ? "verbindbaar" : "niet verbindbaar") +
              (found.candidate ? " | CTEK-kandidaat" : "") + "</li>";
    }
    if (!scanCount) page += "<li>Geen apparaten gezien. Controleer bereik en sluit de CTEK-app.</li>";
    page += "</ul><small>Kopieer het adres en het adrestype naar een vrije plek hieronder. De GATT-service bevestigt CTEK.</small></details>";
  }
  for (size_t slot = 0; slot < ctek::kMaxSensors; ++slot) {
    const auto& state = states[slot]; const auto& sensor = sensors[slot];
    page += "<article><h3>Plek " + String(slot + 1) + " — " + escape(state.configured ? sensor.name : String("vrij")) + "</h3>";
    if (state.configured) {
      page += "<p>" + escape(state.status) + "</p>";
      if (state.hasReading) {
        const auto& sample = state.reading.samples[state.reading.count - 1];
        page += "<p>" + String(sample.voltage, 3) + " V; meting " + String(readingAge(slot)) + " s oud; ";
        page += readingFresh(slot) ? "actueel" : "VEROUDERD";
        if (!state.reading.live || state.reading.currentTemperatureValid) page += "; " + String(sample.temperature, 1) + " °C bij de meting";
        if (state.reading.currentTemperatureValid) page += "; nu " + String(state.reading.currentTemperature, 1) + " °C";
        page += "</p>";
      }
      page += "<small>Pogingen " + String(state.attempts) + "; geslaagd " + String(state.successes) + "; opeenvolgende fouten " + String(state.failures) + "</small>";
      page += "<form method='post' action='/poll'>" + token() + "<input type='hidden' name='slot' value='" + String(slot) + "'><button>Nu uitlezen</button></form>";
      page += "<details><summary>BLE-log</summary><pre>" + escape(state.log) + "</pre></details>";
    }
    page += "<form method='post' action='/sensor'>" + token() + "<input type='hidden' name='slot' value='" + String(slot) + "'>";
    page += input("name", "Naam", sensor.name, "maxlength='40'");
    page += input("address", "Bluetooth-adres, bijvoorbeeld 2C:FC:E4:01:02:03", sensor.address, "required minlength='17' maxlength='17'");
    page += input("address_type", "Adrestype uit de scan (meestal 0)", String(sensor.addressType), "type='number' min='0' max='3' required");
    page += input("code", state.configured ? "Verbindingscode; leeg = behouden bij hetzelfde adres" : "Verbindingscode: 16 tekens, of 11 tekens zonder 40149", "", "type='password' maxlength='64' autocomplete='new-password'");
    page += "<button>Sensor opslaan</button></form>";
    if (state.configured) page += "<form method='post' action='/remove'>" + token() + "<input type='hidden' name='slot' value='" + String(slot) + "'><button>Sensor verwijderen</button></form>";
    page += "</article>";
  }
  page += "<h2>Onderhoud via wifi</h2><a class='button' href='/diagnostics'>Diagnostics downloaden</a> ";
  page += "<form method='post' action='/ota-mode'>" + token() + "<button>Uitlezen pauzeren en firmware bijwerken</button></form>";
  page += "<small>Gegevens blijven op het bord en gaan naar je eigen MQTT-broker. Gebruik deze HTTP-/MQTT-verbindingen op je eigen vertrouwde netwerk. De CTEK-app kan een sensor bezet houden.</small></body></html>";
  web.sendHeader("Cache-Control", "no-store");
  web.send(200, "text/html; charset=utf-8", page);
}

void save() {
  if (!mutation()) return;
  long port, poll;
  String ssid = web.arg("ssid"), wifi = web.arg("wifi_password"), host = web.arg("mqtt_host");
  String user = web.arg("mqtt_user"), password = web.arg("mqtt_password"), admin = web.arg("admin_password");
  char* end = nullptr; String voltageText = web.arg("low_voltage");
  float voltage = strtof(voltageText.c_str(), &end);
  if (ssid.isEmpty() || ssid.length() > 32 || wifi.length() > 63 || host.length() > 128 ||
      user.length() > 64 || password.length() > 128 ||
      !number(web.arg("mqtt_port"), 1, 65535, port) || !number(web.arg("poll_seconds"), 60, 900, poll) ||
      voltageText.isEmpty() || !end || *end || !std::isfinite(voltage) || voltage < 8 || voltage > 16 ||
      (!admin.isEmpty() && (admin.length() < 12 || admin.length() > 63))) {
    error("Ongeldige instellingen. Controleer netwerknamen, poort, interval, spanning en wachtwoordlengte."); return;
  }
  for (char c : host) if (!(isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == ':')) {
    error("Gebruik alleen een IP-adres of hostnaam voor de MQTT-broker, zonder http:// of spaties."); return;
  }
  if (!admin.isEmpty()) for (unsigned char c : admin) if (c < 33 || c > 126) {
    error("Gebruik 12–63 zichtbare ASCII-tekens zonder spaties voor het beheerwachtwoord."); return;
  }
  config.ssid = ssid; config.mqttHost = host; config.mqttPort = port; config.mqttUser = user;
  config.pollSeconds = poll; config.lowVoltage = voltage;
  if (web.arg("open_wifi") == "1") config.wifiPassword = "";
  else if (!wifi.isEmpty()) config.wifiPassword = wifi;
  if (!password.isEmpty()) config.mqttPassword = password;
  if (!admin.isEmpty()) config.adminPassword = admin;
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
  if (workInFlight) page += "<p>De huidige Bluetooth-poging wordt afgerond. Vernieuw deze pagina over enkele seconden.</p>";
  else page += "<p>Uitlezen gepauzeerd. Upload alleen <strong>ctek-esp32-wroom-32-ota.bin</strong>. De factory-bin is voor USB.</p><form method='post' action='/update?csrf=" + csrf + "' enctype='multipart/form-data'><input name='firmware' type='file' accept='.bin' required><button>Firmware uploaden</button></form>";
  page += "<form method='post' action='/resume'>" + token() + "<button>Annuleren en uitlezen hervatten</button></form></body></html>";
  web.send(200, "text/html; charset=utf-8", page);
}
void failUpload(const String& reason) { otaError = reason; if (uploadStarted) Update.abort(); uploadStarted = false; }
void upload() {
  auto& current = web.upload(); otaActivity = millis();
  if (current.status == UPLOAD_FILE_START) {
    uploadAllowed = web.authenticate("admin", config.adminPassword.c_str()) && web.arg("csrf") == csrf && otaMode && !workInFlight;
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
