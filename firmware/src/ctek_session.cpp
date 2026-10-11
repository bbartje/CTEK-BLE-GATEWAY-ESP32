#include "ctek_session.h"

namespace ctek {
SessionResult runSession(Transport& io, const std::string& code, const Metadata* previous) {
  SessionResult result;
  std::string normalized;
  if (!normalizeCode(code, normalized)) { result.error = "Ongeldige verbindingscode"; return result; }
  if (!io.unlock(unlockInput(normalized))) { result.error = "Unlock-write mislukt"; return result; }
  io.wait(2000);  // Matches the Android application's unlock delay.
  uint8_t clock[4]{}, period[1]{}, cursor[2]{};
  size_t clockSize = 0, periodSize = 0, cursorSize = 0;
  bool clockRead = io.read(kClock, clock, sizeof(clock), clockSize);
  if (!clockRead || clockSize != sizeof(clock)) {
    io.wait(300);  // Bounded retry, including Android's transient GATT status 137.
    clockRead = io.read(kClock, clock, sizeof(clock), clockSize);
  }
  if (!clockRead || clockSize != sizeof(clock)) {
    result.error = "Klok-read mislukt na unlock; controleer code, bereik en telefoonverbinding";
    return result;
  }
  if (!io.read(kPeriod, period, sizeof(period), periodSize) ||
      !io.read(kCursor, cursor, sizeof(cursor), cursorSize) ||
      !parseMetadata(clock, clockSize, period, periodSize, cursor, cursorSize, result.metadata)) {
    result.error = "Ongeldige klok, meetperiode of historiecursor";
    return result;
  }
  uint8_t temperature[1]; size_t temperatureSize = 0;
  result.currentTemperatureValid = io.read(kTemperature, temperature, sizeof(temperature), temperatureSize) &&
      decodeTemperature(temperature, temperatureSize, result.currentTemperature);
  if (!hasHistory(result.metadata)) {
    if (result.metadata.uptime <= 6) { result.error = "Sensor net gestart; nog geen metingen"; return result; }
    uint8_t voltage[20]; size_t voltageSize = 0;
    if (!io.read(kLiveVoltage, voltage, sizeof(voltage), voltageSize) ||
        !decodeLiveVoltage(voltage, voltageSize, result.samples[0].voltage)) {
      result.error = "Geen historie en live-spanning niet beschikbaar";
      return result;
    }
    result.live = true;
    result.count = 1;
    result.samples[0].temperature = result.currentTemperature;
  } else {
    result.plan = planHistory(result.metadata, previous);
    if (!io.subscribe()) { result.error = "Meetnotificaties inschakelen mislukt"; return result; }
    for (const Range& range : result.plan.ranges) {
      if (!range.count) continue;
      if (!io.fetch(range, result.samples.data() + result.count, kMaxHistory - result.count)) {
        result.count = 0;  // A partial transfer must never be labelled as the latest measurement.
        result.error = "Historie-opvraag mislukt of onvolledig; zie BLE-log";
        return result;
      }
      result.count += range.count;
    }
  }
  result.success = result.count > 0;
  result.error = result.success ? "Meting ontvangen" : "Geen meting ontvangen";
  return result;
}
}  // namespace ctek
