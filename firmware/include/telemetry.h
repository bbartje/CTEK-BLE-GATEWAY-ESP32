#pragma once
#include "ctek_session.h"

namespace ctek {
struct Telemetry {
  bool fresh = false;
  bool lowVoltage = false;
  bool lowVoltageValid = false;
  bool historicalTemperatureValid = false;
  uint32_t ageSeconds = 0;
  uint64_t sampleTime = 0;
};
inline Telemetry telemetry(const SessionResult& result, uint32_t nowMs,
                           uint32_t readMs, uint64_t readEpoch,
                           float threshold, uint32_t staleSeconds) {
  Telemetry output;
  if (!result.success || !result.count || result.count > kMaxHistory) return output;
  output.ageSeconds = uint32_t(nowMs - readMs) / 1000 +
                      (result.live || result.currentVoltageValid ? 0 : latestAgeSeconds(result.metadata));
  output.fresh = output.ageSeconds < staleSeconds;
  output.lowVoltageValid = result.currentVoltageValid;
  output.lowVoltage = output.lowVoltageValid && result.currentVoltage < threshold;
  output.historicalTemperatureValid = !result.live || result.currentTemperatureValid;
  output.sampleTime = result.live || result.currentVoltageValid ? readEpoch :
      sampleEpoch(readEpoch, result.metadata, result.count, result.count - 1);
  return output;
}
inline uint32_t failureBackoffMs(uint32_t failures) {
  if (!failures) return 0;
  uint32_t shift = failures - 1 > 4 ? 4 : failures - 1;
  uint32_t interval = 30000UL << shift;
  return interval > 300000 ? 300000 : interval;
}
}
