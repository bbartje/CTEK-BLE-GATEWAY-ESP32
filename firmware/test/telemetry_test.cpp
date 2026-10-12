#include "telemetry.h"
#include <cassert>
#include <iostream>
#include <limits>

int main() {
  using namespace ctek;
  assert(rssiAvailable(-60) && rssiAvailable(-90));
  assert(!rssiAvailable(0));    // NimBLE error/default value is not a strong signal.
  assert(!rssiAvailable(127));  // Bluetooth controller's unavailable value.
  SessionResult result;
  assert(!telemetry(result, 0, 0, 0, 12.2f, 1800).fresh);
  result.success = true; result.count = 1; result.metadata = {960, 5, 3};
  result.samples[0] = {12.0f, -10.5f};
  auto value = telemetry(result, 100000, 10000, 2000000000, 12.2f, 1800);
  assert(value.fresh && !value.lowVoltageValid && value.ageSeconds == 150);
  assert(value.sampleTime == 1999999940 && value.historicalTemperatureValid);
  assert(telemetry(result, 1749000, 10000, 0, 12.2f, 1800).fresh);
  assert(!telemetry(result, 1750000, 10000, 0, 12.2f, 1800).fresh);
  assert(telemetry(result, 100000, 10000, 0, 12.2f, 1800).sampleTime == 0);
  result.samples[0].voltage = 12.2f;
  assert(!telemetry(result, 10000, 10000, 0, 12.2f, 1800).lowVoltage);
  // Live readings must take precedence over a different historic voltage/age.
  result.samples[0].voltage = 11.0f;
  result.metadata.periodMinutes = 60;
  result.metadata.uptime = 3500;
  result.currentVoltageValid = true; result.currentVoltage = 13.4f;
  value = telemetry(result, 100000, 10000, 2000000000, 12.2f, 1800);
  assert(value.fresh && value.ageSeconds == 90 && value.sampleTime == 2000000000);
  assert(value.lowVoltageValid && !value.lowVoltage);
  result.currentVoltage = 12.0f;
  assert(telemetry(result, 10000, 10000, 0, 12.2f, 1800).lowVoltage);
  result.currentVoltage = 12.2f;
  assert(!telemetry(result, 10000, 10000, 0, 12.2f, 1800).lowVoltage);
  result.currentVoltageValid = false;
  assert(!telemetry(result, 100000, 10000, 0, 12.2f, 1800).fresh);
  assert(!telemetry(result, 10000, 10000, 0, 12.2f, 1800).lowVoltageValid);
  result.live = true; result.currentTemperatureValid = false;
  value = telemetry(result, 100000, 10000, 2000000000, 12.2f, 1800);
  assert(value.ageSeconds == 90 && value.sampleTime == 2000000000 && !value.historicalTemperatureValid);
  result.currentTemperatureValid = true;
  assert(telemetry(result, 10000, 10000, 0, 12.2f, 1800).historicalTemperatureValid);
  auto beforeWrap = std::numeric_limits<uint32_t>::max() - 999;
  assert(telemetry(result, 2000, beforeWrap, 0, 12.2f, 1800).ageSeconds == 3);
  result.count = 0; assert(!telemetry(result, 0, 0, 0, 12.2f, 1800).fresh);
  assert(failureBackoffMs(1) == 30000 && failureBackoffMs(2) == 60000);
  assert(failureBackoffMs(3) == 120000 && failureBackoffMs(4) == 240000);
  assert(failureBackoffMs(5) == 300000 && failureBackoffMs(999999) == 300000);
  std::cout << "Telemetrietests geslaagd: RSSI-beschikbaarheid, verouderde gegevens, drempel, tijden en fout-backoff.\n";
}
