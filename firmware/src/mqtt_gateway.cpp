#include "gateway_types.h"
#include "telemetry.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

namespace {
class BrokerClient : public WiFiClient {
 public:
  int connect(IPAddress ip, uint16_t port) override { return WiFiClient::connect(ip, port, 1500); }
  int connect(const char* host, uint16_t port) override { return WiFiClient::connect(host, port, 1500); }
};
BrokerClient connection;
PubSubClient mqtt(connection);
uint32_t lastConnect = 0, lastHealth = 0;
bool birthRequested = false;
String rootTopic;

String id(size_t slot) {
  String value = sensors[slot].address;
  value.replace(":", ""); value.toLowerCase(); return value;
}
String sensorRoot(size_t slot) { return rootTopic + "/" + id(slot); }
String discoveryTopic(size_t slot, const char* component, const char* field) {
  return String("homeassistant/") + component + "/ctek_" + gatewayIdentifier + "_" + id(slot) + "/" + field + "/config";
}
bool publishJson(const String& topic, const JsonDocument& doc, bool retained = false) {
  String payload; serializeJson(doc, payload);
  return mqtt.publish(topic.c_str(), payload.c_str(), retained);
}
void deviceInfo(JsonDocument& doc, size_t slot) {
  auto device = doc["device"].to<JsonObject>();
  device["identifiers"].to<JsonArray>().add("ctek_" + gatewayIdentifier + "_" + id(slot));
  device["name"] = sensors[slot].name;
  device["manufacturer"] = "CTEK";
  device["model"] = "Battery Sense via ESP32";
  device["sw_version"] = kFirmwareVersion;
  device["configuration_url"] = "http://" + WiFi.localIP().toString();
}
void availability(JsonDocument& doc, size_t slot) {
  auto list = doc["availability"].to<JsonArray>();
  list.add<JsonObject>()["topic"] = rootTopic + "/availability";
  list.add<JsonObject>()["topic"] = sensorRoot(slot) + "/availability";
  doc["availability_mode"] = "all";
}
bool discovery(size_t slot, const char* field, const char* label,
               const char* unit, const char* deviceClass, bool diagnostic = false) {
  JsonDocument doc;
  doc["name"] = label;
  doc["unique_id"] = "ctek_" + gatewayIdentifier + "_" + id(slot) + "_" + field;
  doc["state_topic"] = sensorRoot(slot) + "/state";
  doc["value_template"] = String("{{ value_json.") + field + " | default(none) }}";
  doc["expire_after"] = kStaleSeconds;
  doc["unit_of_measurement"] = unit;
  doc["device_class"] = deviceClass;
  doc["state_class"] = "measurement";
  if (diagnostic) doc["entity_category"] = "diagnostic";
  availability(doc, slot); deviceInfo(doc, slot);
  return publishJson(discoveryTopic(slot, "sensor", field), doc, true);
}
bool lowDiscovery(size_t slot) {
  JsonDocument doc;
  doc["name"] = "Lage spanning";
  doc["unique_id"] = "ctek_" + gatewayIdentifier + "_" + id(slot) + "_low_voltage";
  doc["state_topic"] = sensorRoot(slot) + "/state";
  doc["value_template"] = "{{ 'ON' if value_json.low_voltage else 'OFF' }}";
  doc["device_class"] = "problem";
  doc["expire_after"] = kStaleSeconds;
  availability(doc, slot); deviceInfo(doc, slot);
  return publishJson(discoveryTopic(slot, "binary_sensor", "low_voltage"), doc, true);
}
bool publishDiscovery(size_t slot) {
  bool success = discovery(slot, "voltage", "Spanning", "V", "voltage");
  success = discovery(slot, "temperature", "Temperatuur bij spanningsmeting", "°C", "temperature") && success;
  success = discovery(slot, "current_temperature", "Actuele temperatuur", "°C", "temperature") && success;
  success = discovery(slot, "rssi", "Bluetooth-signaal", "dBm", "signal_strength", true) && success;
  return lowDiscovery(slot) && success;
}

bool publishReading(size_t slot) {
  const auto& state = states[slot];
  const auto& result = state.reading;
  const auto& latest = result.samples[result.count - 1];
  auto values = ctek::telemetry(result, millis(), state.readMs, state.readEpoch, config.lowVoltage, kStaleSeconds);
  JsonDocument doc;
  doc["voltage"] = latest.voltage;
  if (values.historicalTemperatureValid) doc["temperature"] = latest.temperature;
  else doc["temperature"] = nullptr;
  if (result.currentTemperatureValid) doc["current_temperature"] = result.currentTemperature;
  else doc["current_temperature"] = nullptr;
  doc["low_voltage"] = values.lowVoltage;
  doc["low_voltage_threshold"] = config.lowVoltage;
  doc["rssi"] = state.rssi;
  doc["source"] = result.live ? "live" : "history";
  doc["sample_age_seconds"] = readingAge(slot);
  doc["sample_period_minutes"] = result.metadata.periodMinutes;
  doc["sensor_uptime_seconds"] = result.metadata.uptime;
  doc["cursor"] = result.metadata.cursor;
  doc["history_omitted"] = result.plan.omitted;
  doc["sensor_reset_detected"] = result.plan.reset;
  uint64_t epoch = values.sampleTime;
  if (epoch) doc["sample_time"] = epoch; else doc["sample_time"] = nullptr;
  doc["sample_time_estimated"] = !result.live;
  // Not retained: HA's expire_after must not be reset by replaying an old retained value.
  return publishJson(sensorRoot(slot) + "/state", doc);
}

bool publishHistory(size_t slot) {
  const auto& state = states[slot]; const auto& result = state.reading;
  if (result.live || !result.plan.newCount) return true;
  // Individual records keep each MQTT packet small and preserve sample timestamps.
  for (size_t i = 0; i < result.count; ++i) {
    JsonDocument doc;
    doc["voltage"] = result.samples[i].voltage;
    doc["temperature"] = result.samples[i].temperature;
    doc["cursor"] = (result.metadata.cursor + ctek::kCapacity - result.count + i) % ctek::kCapacity;
    doc["sensor_uptime_seconds"] = result.metadata.uptime;
    uint64_t epoch = ctek::sampleEpoch(state.readEpoch, result.metadata, result.count, i);
    if (epoch) doc["sample_time"] = epoch; else doc["sample_time"] = nullptr;
    doc["sample_time_estimated"] = true;
    if (!publishJson(sensorRoot(slot) + "/history", doc)) return false;
    mqtt.loop();
  }
  return true;
}

void connected() {
  mqtt.publish((rootTopic + "/availability").c_str(), "online", true);
  mqtt.subscribe("homeassistant/status");
  for (size_t slot = 0; slot < ctek::kMaxSensors; ++slot) {
    if (!states[slot].configured) continue;
    states[slot].discoveryPending = true;
    states[slot].availabilityKnown = false;
    if (readingFresh(slot)) states[slot].pending = true;
  }
}
}

void startMqtt() {
  rootTopic = "ctek_gateway/" + gatewayIdentifier;
  mqtt.setServer(config.mqttHost.c_str(), config.mqttPort);
  mqtt.setBufferSize(2048);
  mqtt.setKeepAlive(30);
  mqtt.setSocketTimeout(2);
  mqtt.setCallback([](char* topic, uint8_t* payload, unsigned int length) {
    if (!strcmp(topic, "homeassistant/status") && length == 6 && !memcmp(payload, "online", 6)) birthRequested = true;
  });
}

void serviceMqtt() {
  if (WiFi.status() != WL_CONNECTED || config.mqttHost.isEmpty()) return;
  if (!mqtt.connected()) {
    if (!ctek::elapsed(millis(), lastConnect, 10000)) return;
    lastConnect = millis();
    String clientId = "ctek-" + gatewayIdentifier;
    String will = rootTopic + "/availability";
    bool ok = config.mqttUser.isEmpty() ?
        mqtt.connect(clientId.c_str(), will.c_str(), 0, true, "offline") :
        mqtt.connect(clientId.c_str(), config.mqttUser.c_str(), config.mqttPassword.c_str(), will.c_str(), 0, true, "offline");
    if (!ok) return;
    connected();
  }
  mqtt.loop();
  if (birthRequested) {
    birthRequested = false;
    for (auto& state : states) { state.discoveryPending = true; if (state.hasReading) state.pending = true; }
  }
  for (size_t slot = 0; slot < ctek::kMaxSensors; ++slot) {
    auto& state = states[slot]; if (!state.configured) continue;
    if (state.discoveryPending && publishDiscovery(slot)) state.discoveryPending = false;
    bool fresh = readingFresh(slot);
    if (!state.availabilityKnown || state.onlinePublished != fresh) {
      if (mqtt.publish((sensorRoot(slot) + "/availability").c_str(), fresh ? "online" : "offline", true)) {
        state.availabilityKnown = true; state.onlinePublished = fresh;
      }
    }
    if (state.pending && fresh && publishReading(slot) && publishHistory(slot)) state.pending = false;
  }
  if (ctek::elapsed(millis(), lastHealth, 60000)) {
    lastHealth = millis();
    JsonDocument doc;
    doc["firmware"] = kFirmwareVersion; doc["uptime_seconds"] = millis() / 1000;
    doc["free_heap"] = ESP.getFreeHeap(); doc["min_free_heap"] = ESP.getMinFreeHeap();
    doc["wifi_rssi"] = WiFi.RSSI(); doc["time_synchronized"] = validEpoch() != 0;
    publishJson(rootTopic + "/diagnostics", doc);
  }
}

void removeDiscovery(size_t slot) {
  if (!mqtt.connected()) return;
  for (const char* field : {"voltage", "temperature", "current_temperature", "rssi"})
    mqtt.publish(discoveryTopic(slot, "sensor", field).c_str(), "", true);
  mqtt.publish(discoveryTopic(slot, "binary_sensor", "low_voltage").c_str(), "", true);
  mqtt.publish((sensorRoot(slot) + "/availability").c_str(), "", true);
}
bool mqttConnected() { return mqtt.connected(); }
int mqttStatus() { return mqtt.state(); }
