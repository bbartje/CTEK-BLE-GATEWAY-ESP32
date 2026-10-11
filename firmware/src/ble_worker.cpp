#include "gateway_types.h"

#include <MD5Builder.h>
#include <NimBLEDevice.h>
#include <cstdarg>
#include <cstring>
#include <ctime>

namespace {
QueueHandle_t jobs = nullptr, reports = nullptr;
portMUX_TYPE notificationMux = portMUX_INITIALIZER_UNLOCKED;
struct NotificationBuffer {
  ctek::Sample samples[ctek::kMaxHistory]{};
  uint16_t expected = 0, count = 0, handle = BLE_HS_CONN_HANDLE_NONE;
  bool active = false, invalid = false;
} received;

void appendLog(BleReport& report, const char* format, ...) {
  size_t used = strlen(report.log);
  if (used >= sizeof(report.log) - 2) return;
  va_list args;
  va_start(args, format);
  vsnprintf(report.log + used, sizeof(report.log) - used, format, args);
  va_end(args);
}

void notifications(NimBLERemoteCharacteristic* characteristic, uint8_t* data, size_t length, bool) {
  // No Serial, String, MQTT or web calls in the BLE host callback.
  uint16_t handle = characteristic->getClient()->getConnHandle();
  portENTER_CRITICAL(&notificationMux);
  if (received.active && received.handle == handle) {
    size_t count = 0;
    if (!ctek::decodeSamples(data, length, received.samples + received.count,
                            received.expected - received.count, count)) {
      received.invalid = true;
    } else {
      received.count += count;
    }
  }
  portEXIT_CRITICAL(&notificationMux);
}

class NimbleTransport : public ctek::Transport {
 public:
  NimbleTransport(NimBLEClient* client, NimBLERemoteService* service, BleReport& report)
      : client_(client), service_(service), report_(report) {}

  bool unlock(const std::array<uint8_t, 32>& input) override {
    auto* characteristic = service_->getCharacteristic(ctek::kUnlock);
    if (!characteristic || !characteristic->canWrite()) {
      appendLog(report_, "unlock: leespunt %s; code niet verstuurd\n",
                characteristic ? "biedt geen schrijftoegang" : "niet gevonden");
      return false;
    }
    MD5Builder md5; md5.begin();
    md5.add(const_cast<uint8_t*>(input.data()), input.size());
    md5.calculate();
    uint8_t digest[16]; md5.getBytes(digest);
    bool ok = characteristic->writeValue(digest, sizeof(digest), true);
    memset(digest, 0, sizeof(digest));
    appendLog(report_, "unlock write: %s (code en hash worden niet gelogd)\n", ok ? "OK" : "mislukt");
    if (ok) appendLog(report_, "Ontgrendelbericht verstuurd; code nog niet bevestigd\n");
    return ok;
  }

  bool read(const char* uuid, uint8_t* output, size_t capacity, size_t& length) override {
    length = 0;
    readError_ = nullptr;
    if (!client_->isConnected()) {
      readError_ = "Bluetooth-verbinding verbroken voor uitlezen; zie BLE-log";
      appendLog(report_, "read %s: verbinding verbroken\n", uuid);
      return false;
    }
    auto* characteristic = service_->getCharacteristic(uuid);
    if (!characteristic) {
      readError_ = "Bluetooth-leespunt niet gevonden; zie BLE-log";
      appendLog(report_, "read %s: leespunt niet gevonden; connected=%u\n", uuid, client_->isConnected());
      return false;
    }
    if (!characteristic->canRead()) {
      readError_ = "Bluetooth-leespunt biedt geen leestoegang; zie BLE-log";
      appendLog(report_, "read %s: leespunt gevonden, maar read=0\n", uuid);
      return false;
    }
    time_t completedAt = -1;
    NimBLEAttValue value = characteristic->readValue(&completedAt);
    // NimBLE's client error is a connection error, not the GATT read status.
    // A timestamp is written only when readValue completes successfully.
    appendLog(report_, "read %s: len=%u completed=%u raw=", uuid, unsigned(value.size()), completedAt != -1);
    for (size_t i = 0; i < value.size() && i < 20; ++i) appendLog(report_, "%02X ", value[i]);
    appendLog(report_, "\n");
    if (completedAt == -1) {
      readError_ = "Bluetooth-read mislukt; controleer code, bereik en telefoon; zie BLE-log";
      appendLog(report_, "read niet voltooid; connected=%u (verbindingsstatus is geen GATT-readstatus)\n",
                client_->isConnected());
      return false;
    }
    if (!value.size() || value.size() > capacity) {
      readError_ = "Bluetooth-read gaf een ongeldige lengte; zie BLE-log";
      return false;
    }
    length = value.size();
    memcpy(output, value.data(), length);
    if (!strcmp(uuid, ctek::kCursor) || !strcmp(uuid, ctek::kLiveVoltage)) {
      report_.readMs = millis();
      report_.readEpoch = validEpoch();
    }
    return true;
  }
  const char* readError() const override { return readError_; }

  bool subscribe() override {
    auto* samples = service_->getCharacteristic(ctek::kSamples);
    bool ok = samples && samples->canNotify() && samples->subscribe(true, notifications, true);
    appendLog(report_, "subscribe + CCCD write: %s\n", ok ? "OK" : "mislukt");
    return ok;
  }

  bool fetch(const ctek::Range& range, ctek::Sample* output, size_t capacity) override {
    auto* characteristic = service_->getCharacteristic(ctek::kRange);
    if (!characteristic || !characteristic->canWrite() || range.count > capacity) return false;
    portENTER_CRITICAL(&notificationMux);
    received.expected = range.count;
    received.count = 0;
    received.invalid = false;
    received.handle = client_->getConnHandle();
    received.active = true;
    portEXIT_CRITICAL(&notificationMux);
    auto payload = ctek::rangePayload(range);
    bool written = characteristic->writeValue(payload.data(), payload.size(), true);
    uint32_t started = millis();
    uint16_t count = 0; bool invalid = false;
    while (written && client_->isConnected() && !ctek::elapsed(millis(), started, 15000)) {
      portENTER_CRITICAL(&notificationMux);
      count = received.count; invalid = received.invalid;
      portEXIT_CRITICAL(&notificationMux);
      if (invalid || count == range.count) break;
      vTaskDelay(pdMS_TO_TICKS(20));
    }
    portENTER_CRITICAL(&notificationMux);
    count = received.count; invalid = received.invalid;
    received.active = false;
    bool complete = written && !invalid && count == range.count;
    if (complete) memcpy(output, received.samples, count * sizeof(ctek::Sample));
    portEXIT_CRITICAL(&notificationMux);
    appendLog(report_, "range first=%u count=%u write=%u received=%u invalid=%u\n",
              range.first, range.count, written, count, invalid);
    if (complete) {
      const auto& last = output[count - 1];
      appendLog(report_, "laatste record: %.4f V, %.1f C\n", last.voltage, last.temperature);
    }
    return complete;
  }
  void wait(uint32_t duration) override { vTaskDelay(pdMS_TO_TICKS(duration)); }
 private:
  NimBLEClient* client_;
  NimBLERemoteService* service_;
  BleReport& report_;
  const char* readError_ = nullptr;
};

void scan(BleReport& report) {
  NimBLEScan* scanner = NimBLEDevice::getScan();
  scanner->clearResults();
  NimBLEScanResults found = scanner->getResults(6000, false);
  for (int i = 0; i < found.getCount() && report.found < kScanLimit; ++i) {
    const NimBLEAdvertisedDevice* device = found.getDevice(i);
    auto& entry = report.devices[report.found++];
    snprintf(entry.address, sizeof(entry.address), "%s", device->getAddress().toString().c_str());
    snprintf(entry.name, sizeof(entry.name), "%s", device->getName().c_str());
    entry.addressType = device->getAddress().getType();
    entry.rssi = device->getRSSI();
    entry.connectable = device->isConnectable();
    entry.candidate = device->isAdvertisingService(NimBLEUUID(ctek::kService)) ||
                      String(entry.address).startsWith("2c:fc:e4");
  }
  scanner->clearResults();
}

void readSensor(const BleJob& job, BleReport& report) {
  appendLog(report, "Firmware %s; adres=%s type=%u\n", kFirmwareVersion, job.address, job.addressType);
  auto* client = NimBLEDevice::createClient();
  if (!client) { report.result.error = "BLE-client aanmaken mislukt"; return; }
  client->setConnectTimeout(7000);
  // 30-60 ms, no slave latency; one connection and MTU 23 like the original app.
  client->setConnectionParams(24, 48, 0, 400);
  bool connected = client->connect(NimBLEAddress(std::string(job.address), job.addressType), true, false, false);
  report.error = client->getLastError();
  appendLog(report, "connect=%u status=%d (%s)\n", connected, report.error,
            NimBLEUtils::returnCodeToString(report.error));
  if (connected) {
    vTaskDelay(pdMS_TO_TICKS(800));  // Android also lets the connection settle before GATT discovery.
    report.rssi = client->getRssi();
    auto* service = client->getService(ctek::kService);
    if (!service) {
      report.result.error = "Verbonden, maar CTEK-service ontbreekt";
      for (auto* found : client->getServices()) appendLog(report, "gevonden service: %s\n", found->getUUID().toString().c_str());
    } else {
      // NimBLE 2.x defaults to cache-only access. Explicitly discover all points
      // before using UUID lookups, and include the sensor's read/write properties.
      const auto& characteristics = service->getCharacteristics(true);
      appendLog(report, "GATT-discovery: %u punten; connected=%u\n",
                unsigned(characteristics.size()), client->isConnected());
      for (auto* characteristic : characteristics) {
        appendLog(report, "GATT %s read=%u write=%u notify=%u\n",
                  characteristic->getUUID().toString().c_str(), characteristic->canRead(),
                  characteristic->canWrite(), characteristic->canNotify());
      }
      if (characteristics.empty()) {
        report.result.error = "CTEK-leespunten niet gevonden; code niet verstuurd; zie BLE-log";
      } else {
        NimbleTransport transport(client, service, report);
        report.result = ctek::runSession(transport, job.code, job.previousValid ? &job.previous : nullptr);
      }
    }
  } else {
    report.result.error = "BLE-verbinding mislukt; code niet verstuurd; controleer bereik, adrestype en telefoon";
  }
  portENTER_CRITICAL(&notificationMux);
  received.active = false;
  portEXIT_CRITICAL(&notificationMux);
  // deleteClient owns asynchronous disconnect cleanup; do not disconnect twice.
  bool deleted = NimBLEDevice::deleteClient(client);
  appendLog(report, "%s; client opgeruimd=%u\n", report.result.error, deleted);
}

void worker(void*) {
  NimBLEDevice::init("CTEK-Gateway");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setMTU(23);
  NimBLEScan* scanner = NimBLEDevice::getScan();
  scanner->setActiveScan(true);
  scanner->setInterval(80);
  scanner->setWindow(30);
  scanner->setMaxResults(kScanLimit);
  // Large report is heap-allocated once; queue copies only POD data.
  auto* report = new BleReport;
  BleJob job;
  for (;;) {
    if (xQueueReceive(jobs, &job, portMAX_DELAY) != pdTRUE) continue;
    *report = BleReport{};
    report->kind = job.kind; report->slot = job.slot; report->generation = job.generation;
    if (job.kind == BleJobKind::Scan) scan(*report);
    else readSensor(job, *report);
    memset(job.code, 0, sizeof(job.code));
    xQueueSend(reports, report, portMAX_DELAY);
  }
}
}  // namespace

bool startBleWorker() {
  jobs = xQueueCreate(1, sizeof(BleJob)); reports = xQueueCreate(1, sizeof(BleReport));
  return jobs && reports && xTaskCreatePinnedToCore(worker, "ctek_ble", 12288,
          nullptr, 1, nullptr, 1) == pdPASS;
}
bool enqueueBle(const BleJob& job) { return jobs && xQueueSend(jobs, &job, 0) == pdTRUE; }
bool receiveBle(BleReport& report) { return reports && xQueueReceive(reports, &report, 0) == pdTRUE; }
