#include "gateway_types.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_system.h>
#include <cstring>

namespace {
Preferences preferences;
bool apStarted = false, wasConnected = false;
uint32_t lastReconnect = 0, disconnectedSince = 0;
String hostname;

void openSetup() {
  if (apStarted) return;
  WiFi.mode(WIFI_AP_STA);
  apStarted = WiFi.softAP(setupSsid.c_str(), config.adminPassword.c_str(), 1, false, 1);
  Serial.printf("Setup-wifi: %s; adres http://192.168.4.1; WPA2-wachtwoord = beheerwachtwoord\n", setupSsid.c_str());
}
String generatedPassword() {
  constexpr char alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789";
  String password;
  for (size_t i = 0; i < 16; ++i) password += alphabet[esp_random() % (sizeof(alphabet) - 1)];
  return password;
}
}

void loadConfiguration() {
  preferences.begin("ctek-gw", false);
  config.ssid = preferences.getString("ssid", "");
  config.wifiPassword = preferences.getString("wifi_pw", "");
  config.mqttHost = preferences.getString("mqtt_host", "");
  config.mqttPort = preferences.getUShort("mqtt_port", 1883);
  config.mqttUser = preferences.getString("mqtt_user", "");
  config.mqttPassword = preferences.getString("mqtt_pw", "");
  config.pollSeconds = preferences.getUInt("poll_s", 300);
  if (config.pollSeconds < 60 || config.pollSeconds > 900) config.pollSeconds = 300;
  config.lowVoltage = preferences.getFloat("low_v", 12.2f);
  if (!(config.lowVoltage >= 8 && config.lowVoltage <= 16)) config.lowVoltage = 12.2f;
  config.adminPassword = preferences.getString("admin_pw", "");
  if (config.adminPassword.length() < 12 || config.adminPassword.length() > 63) {
    config.adminPassword = generatedPassword();
    preferences.putString("admin_pw", config.adminPassword);
  }
  JsonDocument stored;
  if (deserializeJson(stored, preferences.getString("sensors", "[]"))) return;
  size_t slot = 0;
  for (JsonObjectConst item : stored.as<JsonArrayConst>()) {
    if (slot >= ctek::kMaxSensors) break;
    std::string address, code;
    const char* savedAddress = item["address"] | "";
    const char* savedCode = item["code"] | "";
    int type = item["address_type"] | 0;
    if (!ctek::normalizeAddress(savedAddress, address) || !ctek::normalizeCode(savedCode, code) || type < 0 || type > 3) continue;
    bool duplicate = false;
    for (size_t i = 0; i < slot; ++i) if (sensors[i].address == address.c_str()) duplicate = true;
    if (duplicate) continue;
    sensors[slot].address = address.c_str(); sensors[slot].code = code.c_str();
    sensors[slot].name = item["name"] | address.c_str();
    if (sensors[slot].name.length() > 40) sensors[slot].name = sensors[slot].name.substring(0, 40);
    sensors[slot].addressType = type;
    states[slot++].configured = true;
  }
}
void saveConfiguration() {
  preferences.putString("ssid", config.ssid);
  preferences.putString("wifi_pw", config.wifiPassword);
  preferences.putString("mqtt_host", config.mqttHost);
  preferences.putUShort("mqtt_port", config.mqttPort);
  preferences.putString("mqtt_user", config.mqttUser);
  preferences.putString("mqtt_pw", config.mqttPassword);
  preferences.putUInt("poll_s", config.pollSeconds);
  preferences.putFloat("low_v", config.lowVoltage);
  preferences.putString("admin_pw", config.adminPassword);
}
void saveSensors() {
  JsonDocument doc;
  auto list = doc.to<JsonArray>();
  for (size_t slot = 0; slot < ctek::kMaxSensors; ++slot) {
    if (!states[slot].configured) continue;
    auto item = list.add<JsonObject>();
    item["address"] = sensors[slot].address;
    item["address_type"] = sensors[slot].addressType;
    item["name"] = sensors[slot].name;
    item["code"] = sensors[slot].code;
  }
  String text; serializeJson(doc, text);
  preferences.putString("sensors", text);
}

void startNetwork() {
  WiFi.mode(WIFI_STA);
  gatewayIdentifier = WiFi.macAddress(); gatewayIdentifier.replace(":", ""); gatewayIdentifier.toLowerCase();
  hostname = "ctek-" + gatewayIdentifier.substring(6);
  setupSsid = "CTEK-Setup-" + gatewayIdentifier.substring(6);
  WiFi.setHostname(hostname.c_str());
  WiFi.setAutoReconnect(true);
  // Configuration is stored with Preferences; avoid writing Wi-Fi state on every retry.
  WiFi.persistent(false);
  Serial.printf("Beheerlogin: admin; uniek wachtwoord: %s\n", config.adminPassword.c_str());
  Serial.println("Bewaar dit wachtwoord lokaal; deel deze regel niet in diagnostics.");
  if (config.ssid.isEmpty()) openSetup();
  else WiFi.begin(config.ssid.c_str(), config.wifiPassword.c_str());
  disconnectedSince = lastReconnect = millis();
}

void serviceNetwork() {
  bool connected = WiFi.status() == WL_CONNECTED;
  if (connected && !wasConnected) {
    Serial.printf("Wifi verbonden; configuratie http://%s of http://%s.local\n",
                  WiFi.localIP().toString().c_str(), hostname.c_str());
    MDNS.end();
    if (MDNS.begin(hostname.c_str())) MDNS.addService("http", "tcp", 80);
    configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");
    if (apStarted) { WiFi.softAPdisconnect(true); WiFi.mode(WIFI_STA); apStarted = false; }
  }
  if (!connected) {
    if (wasConnected) disconnectedSince = millis();
    if (!config.ssid.isEmpty() && ctek::elapsed(millis(), lastReconnect, 30000)) {
      lastReconnect = millis(); WiFi.reconnect();
    }
    if (ctek::elapsed(millis(), disconnectedSince, 60000)) openSetup();
  }
  wasConnected = connected;
}
