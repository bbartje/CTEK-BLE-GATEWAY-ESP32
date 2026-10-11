#include "gateway_types.h"
#include "telemetry.h"
#include <WiFi.h>
#include <ctime>
#include <algorithm>
#include <cstring>

GatewayConfig config;
SensorConfig sensors[ctek::kMaxSensors];
SensorState states[ctek::kMaxSensors];
ScanEntry scanEntries[kScanLimit];
uint8_t scanCount = 0;
bool scanRequested = false, scanFinished = false, workInFlight = false, otaMode = false;
String gatewayIdentifier, setupSsid;

uint64_t validEpoch() {
  time_t now = time(nullptr);
  return now >= 1735689600 ? uint64_t(now) : 0;  // No fabricated dates before NTP sync.
}
uint32_t readingAge(size_t slot) {
  const auto& state = states[slot];
  return ctek::telemetry(state.reading, millis(), state.readMs, state.readEpoch,
                         config.lowVoltage, kStaleSeconds).ageSeconds;
}
bool readingFresh(size_t slot) { return states[slot].hasReading && readingAge(slot) < kStaleSeconds; }

namespace {
bool bleReady = false;
size_t nextSlot = 0;
uint32_t lastDispatch = 0;
BleReport report;

void processReport() {
  if (!receiveBle(report)) return;
  workInFlight = false;
  if (report.kind == BleJobKind::Scan) {
    scanCount = report.found;
    memcpy(scanEntries, report.devices, sizeof(scanEntries));
    scanFinished = true;
    return;
  }
  size_t slot = report.slot;
  auto& state = states[slot];
  if (!state.configured || state.generation != report.generation) return;
  state.lastAttempt = millis();  // Backoff starts after completion, including long GATT timeouts.
  state.status = report.result.error;
  state.log = report.log;
  if (report.error != 0) state.status += String(" (BLE-verbindingsstatus ") + report.error + ")";
  if (report.result.success) {
    state.reading = report.result;
    state.readMs = report.readMs;
    state.readEpoch = report.readEpoch;
    state.rssi = report.rssi;
    state.hasReading = state.pending = true;
    state.failures = 0;
    ++state.successes;
    state.retryMs = config.pollSeconds * 1000;
    state.previous = report.result.metadata;
    state.previousValid = !report.result.live;
  } else {
    ++state.failures;
    state.retryMs = ctek::failureBackoffMs(state.failures);
  }
}

void dispatch() {
  if (!bleReady || initialSetupPending() || workInFlight || otaMode || !ctek::elapsed(millis(), lastDispatch, 1000)) return;
  BleJob job;
  if (scanRequested) {
    job.kind = BleJobKind::Scan;
    if (enqueueBle(job)) { workInFlight = true; scanRequested = false; lastDispatch = millis(); }
    return;
  }
  for (size_t checked = 0; checked < ctek::kMaxSensors; ++checked) {
    size_t slot = nextSlot++ % ctek::kMaxSensors;
    auto& state = states[slot];
    if (!state.configured || (state.attempts && !ctek::elapsed(millis(), state.lastAttempt, state.retryMs))) continue;
    job.slot = slot; job.generation = state.generation; job.addressType = sensors[slot].addressType;
    snprintf(job.address, sizeof(job.address), "%s", sensors[slot].address.c_str());
    snprintf(job.code, sizeof(job.code), "%s", sensors[slot].code.c_str());
    job.previousValid = state.previousValid; job.previous = state.previous;
    if (enqueueBle(job)) {
      workInFlight = true; lastDispatch = state.lastAttempt = millis();
      ++state.attempts; state.status = "Bezig met BLE-uitlezing";
    }
    memset(job.code, 0, sizeof(job.code));
    return;
  }
}
}

void setup() {
  Serial.begin(115200);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);  // Enable RF entropy for the form nonce; no station connection yet.
  loadConfiguration();
  startNetwork();
  startMqtt();
  startWeb();
  bleReady = !initialSetupPending() && startBleWorker();
  Serial.printf("CTEK gateway %s; ESP32, flash=%u bytes; BLE-taak=%s\n",
                kFirmwareVersion, ESP.getFlashChipSize(), initialSetupPending() ? "wacht op eigen wachtwoord" : (bleReady ? "gestart" : "MISLUKT"));
}
void loop() {
  processReport();
  serviceNetwork();
  serviceWeb();
  if (!otaMode && !initialSetupPending()) serviceMqtt();
  dispatch();
  delay(2);
}
