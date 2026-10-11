#pragma once
#include "ctek_protocol.h"

namespace ctek {
// The same session logic is used with NimBLE on ESP32 and a scripted transport in tests.
class Transport {
 public:
  virtual ~Transport() = default;
  virtual bool unlock(const std::array<uint8_t, 32>& input) = 0;
  virtual bool read(const char* uuid, uint8_t* output, size_t capacity, size_t& length) = 0;
  // Return only static text: SessionResult outlives the transport and BLE client.
  virtual const char* readError() const { return nullptr; }
  virtual bool subscribe() = 0;
  virtual bool fetch(const Range& range, Sample* output, size_t capacity) = 0;
  virtual void wait(uint32_t milliseconds) = 0;
};
struct SessionResult {
  bool success = false;
  bool live = false;
  bool currentTemperatureValid = false;
  float currentTemperature = 0;
  Metadata metadata{};
  HistoryPlan plan{};
  std::array<Sample, kMaxHistory> samples{};
  uint16_t count = 0;
  const char* error = "Nog geen uitlezing";
};
SessionResult runSession(Transport& transport, const std::string& code,
                         const Metadata* previous);
}  // namespace ctek
