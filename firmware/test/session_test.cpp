#include "ctek_session.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

struct ScriptedTransport : ctek::Transport {
  ctek::Metadata metadata{9000360, 5, 1};
  bool unlockOk = true, clockOk = true, periodOk = true, subscribeOk = true, fetchOk = true;
  bool temperatureOk = true, voltageOk = true, retryClock = false;
  const char* readFailure = nullptr;
  const char* lastReadFailure = nullptr;
  int reads = 0;
  std::vector<std::string> operations;
  std::vector<ctek::Range> requests;
  bool unlock(const std::array<uint8_t,32>& input) override {
    operations.push_back("unlock");
    assert(input[0] == 0xe1 && input[16] == 'A' && input[31] == 'P');
    return unlockOk;
  }
  bool read(const char* uuid, uint8_t* data, size_t capacity, size_t& length) override {
    operations.push_back(uuid); length = 0;
    // A successful read clears an earlier failure, including an optional temperature read.
    lastReadFailure = readFailure;
    if (std::string(uuid) == ctek::kClock) {
      if (!clockOk || (retryClock && reads++ == 0)) return false;
      assert(capacity >= 4); length = 4;
      for (size_t i = 0; i < 4; ++i) data[i] = metadata.uptime >> (8 * i);
    } else if (std::string(uuid) == ctek::kPeriod) {
      if (!periodOk) return false;
      assert(capacity >= 1); length = 1; data[0] = metadata.periodMinutes;
    } else if (std::string(uuid) == ctek::kCursor) {
      assert(capacity >= 2); length = 2; data[0] = metadata.cursor; data[1] = metadata.cursor >> 8;
    } else if (std::string(uuid) == ctek::kTemperature) {
      if (!temperatureOk) return false;
      assert(capacity >= 1); length = 1; data[0] = 41;
    } else if (std::string(uuid) == ctek::kLiveVoltage) {
      if (!voltageOk) return false;
      assert(capacity >= 2); length = 2; data[0] = 0; data[1] = 0x64;
    } else { assert(false); }
    lastReadFailure = nullptr;
    return true;
  }
  const char* readError() const override { return lastReadFailure; }
  bool subscribe() override { operations.push_back("subscribe"); return subscribeOk; }
  bool fetch(const ctek::Range& range, ctek::Sample* out, size_t capacity) override {
    operations.push_back("fetch"); requests.push_back(range);
    assert(range.count <= capacity);
    for (size_t i = 0; i < range.count; ++i) out[i] = {12.5f, -10.5f};
    return fetchOk;
  }
  void wait(uint32_t duration) override { operations.push_back("wait:" + std::to_string(duration)); }
};

int main() {
  using namespace ctek;
  const std::string code = "abcdefghijklmnop";
  Metadata previous{8999460, 5, 29998};
  ScriptedTransport io;
  auto result = runSession(io, code, &previous);
  assert(result.success && result.count == 3 && !result.live);
  assert(result.currentVoltageValid && result.currentVoltage == 12.5f);
  assert(result.currentTemperatureValid && result.currentTemperature == 20.5f);
  assert(result.samples[2].temperature == -10.5f);
  assert(io.operations[0] == "unlock" && io.operations[1] == "wait:2000");
  assert(io.requests.size() == 2 && io.requests[0].first == 29998 && io.requests[1].first == 0);

  ScriptedTransport badCode;
  assert(!runSession(badCode, "short", nullptr).success && badCode.operations.empty());
  ScriptedTransport badUnlock; badUnlock.unlockOk = false;
  assert(!runSession(badUnlock, code, nullptr).success && badUnlock.operations.size() == 1);
  ScriptedTransport locked; locked.clockOk = false;
  assert(!runSession(locked, code, nullptr).success && locked.requests.empty());
  assert(locked.operations.size() == 5);  // Write success is not authentication success; one retry only.
  for (const char* detail : {"Bluetooth-leespunt niet gevonden; zie BLE-log",
                            "Bluetooth-leespunt biedt geen leestoegang; zie BLE-log",
                            "Bluetooth-verbinding verbroken voor uitlezen; zie BLE-log"}) {
    ScriptedTransport inaccessible; inaccessible.clockOk = false; inaccessible.readFailure = detail;
    result = runSession(inaccessible, code, nullptr);
    assert(!result.success && result.count == 0 && std::string(result.error) == detail);
    assert(inaccessible.operations.size() == 5 && inaccessible.requests.empty());
  }
  ScriptedTransport retry; retry.retryClock = true;
  assert(runSession(retry, code, nullptr).success && retry.reads == 2);
  ScriptedTransport retryDetail; retryDetail.retryClock = true;
  retryDetail.readFailure = "Tijdelijke Bluetooth-readfout";
  result = runSession(retryDetail, code, nullptr);
  assert(result.success && std::string(result.error) == "Meting ontvangen" && retryDetail.readError() == nullptr);
  ScriptedTransport badPeriod; badPeriod.periodOk = false;
  assert(!runSession(badPeriod, code, nullptr).success);
  ScriptedTransport missingPeriod; missingPeriod.periodOk = false;
  missingPeriod.readFailure = "Bluetooth-leespunt niet gevonden; zie BLE-log";
  result = runSession(missingPeriod, code, nullptr);
  assert(!result.success && std::string(result.error) == missingPeriod.readFailure);
  ScriptedTransport invalid; invalid.metadata.cursor = 30000;
  assert(!runSession(invalid, code, nullptr).success);
  ScriptedTransport noNotify; noNotify.subscribeOk = false;
  assert(!runSession(noNotify, code, nullptr).success && noNotify.requests.empty());
  ScriptedTransport partial; partial.fetchOk = false;
  result = runSession(partial, code, &previous);
  assert(!result.success && result.count == 0);
  ScriptedTransport noTemperature; noTemperature.temperatureOk = false;
  result = runSession(noTemperature, code, nullptr);
  assert(result.success && !result.currentTemperatureValid);
  ScriptedTransport historyNoLive; historyNoLive.voltageOk = false;
  result = runSession(historyNoLive, code, nullptr);
  assert(result.success && !result.currentVoltageValid && result.count > 0);
  ScriptedTransport live; live.metadata = {30, 5, 0};
  result = runSession(live, code, nullptr);
  assert(result.success && result.live && result.currentVoltageValid && result.currentVoltage == 12.5f);
  assert(result.samples[0].voltage == 12.5f && live.requests.empty());
  ScriptedTransport newborn; newborn.metadata = {6, 5, 0};
  assert(!runSession(newborn, code, nullptr).success);
  ScriptedTransport noLive; noLive.metadata = {30, 5, 0}; noLive.voltageOk = false;
  assert(!runSession(noLive, code, nullptr).success);
  ScriptedTransport noLiveDetail; noLiveDetail.metadata = {30, 5, 0}; noLiveDetail.voltageOk = false;
  noLiveDetail.readFailure = "Bluetooth-read mislukt; zie BLE-log";
  result = runSession(noLiveDetail, code, nullptr);
  assert(!result.success && std::string(result.error) == noLiveDetail.readFailure);
  ScriptedTransport optionalFailure; optionalFailure.metadata = {30, 5, 0}; optionalFailure.temperatureOk = false;
  optionalFailure.readFailure = "Bluetooth-leespunt niet gevonden; zie BLE-log";
  result = runSession(optionalFailure, code, nullptr);
  assert(result.success && result.live && result.currentVoltageValid && !result.currentTemperatureValid);
  assert(optionalFailure.readError() == nullptr);
  std::cout << "Sessietests geslaagd: volgorde, unlockbevestiging, foutdetails, retries, historie en live-fallback.\n";
}
