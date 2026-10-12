#pragma once
#include <Arduino.h>
#include "ctek_session.h"

constexpr char kFirmwareVersion[] = "0.2.0-beta.7";
constexpr uint32_t kStaleSeconds = 1800;
constexpr size_t kScanLimit = 24;

struct SensorConfig {
  String address, name, code;
  uint8_t addressType = 0;
};
struct GatewayConfig {
  String ssid, wifiPassword, mqttHost, mqttUser, mqttPassword, adminPassword, timezone;
  uint16_t mqttPort = 1883;
  uint32_t pollSeconds = 300;
  float lowVoltage = 12.2f;
};
struct SensorState {
  bool configured = false, hasReading = false, pending = false, discoveryPending = true;
  bool onlinePublished = false, availabilityKnown = false;
  uint32_t generation = 1, lastAttempt = 0, retryMs = 0, failures = 0;
  uint32_t readMs = 0, attempts = 0, successes = 0;
  uint64_t readEpoch = 0;
  bool previousValid = false;
  ctek::Metadata previous{};
  ctek::SessionResult reading{};
  int rssi = 0;
  String status = "Nog geen uitlezing", log;
};
enum class BleJobKind : uint8_t { Read, Scan };
struct BleJob {
  BleJobKind kind = BleJobKind::Read;
  uint8_t slot = 0, addressType = 0;
  uint32_t generation = 0;
  char address[18]{}, code[17]{};
  bool previousValid = false;
  ctek::Metadata previous{};
};
struct ScanEntry {
  char address[18]{}, name[33]{};
  uint8_t addressType = 0;
  int rssi = 0;
  bool candidate = false, connectable = false;
};
struct BleReport {
  BleJobKind kind = BleJobKind::Read;
  uint8_t slot = 0;
  uint32_t generation = 0, readMs = 0;
  uint64_t readEpoch = 0;
  int rssi = 0, error = 0;
  ctek::SessionResult result{};
  char log[3072]{};
  uint8_t found = 0;
  ScanEntry devices[kScanLimit]{};
};

extern GatewayConfig config;
extern SensorConfig sensors[ctek::kMaxSensors];
extern SensorState states[ctek::kMaxSensors];
extern ScanEntry scanEntries[kScanLimit];
extern uint8_t scanCount;
extern bool scanRequested, scanFinished, workInFlight, otaMode;
extern String gatewayIdentifier, setupSsid;

bool startBleWorker();
bool enqueueBle(const BleJob& job);
bool receiveBle(BleReport& report);
void loadConfiguration();
void saveConfiguration();
bool initialSetupPending();
bool recoveryAccessOpen();
bool saveAdminPassword(const String& password);
void saveSensors();
void startNetwork();
void serviceNetwork();
void startWeb();
void serviceWeb();
void startMqtt();
void serviceMqtt();
void removeDiscovery(size_t slot);
bool mqttConnected();
int mqttStatus();
bool readingFresh(size_t slot);
uint32_t readingAge(size_t slot);
uint64_t validEpoch();
