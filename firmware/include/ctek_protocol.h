#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace ctek {
constexpr char kService[] = "812ca8ed-a177-4d74-aefa-70098c5416af";
constexpr char kUnlock[] = "a1d67c01-fe3f-01bf-7d49-7dbdc43b02a6";
constexpr char kClock[] = "337ea83e-744b-4b7c-b23f-45beaab11db7";
constexpr char kPeriod[] = "e5a7e9a0-e2d7-4f46-83d1-0e55d2427780";
constexpr char kCursor[] = "fc3119da-9b40-414e-a1c4-6c1737718cae";
constexpr char kRange[] = "418d1817-f516-45f1-ab64-433ae596613d";
constexpr char kSamples[] = "a337b4eb-2544-4f94-87c9-f7732e5fe250";
constexpr char kLiveVoltage[] = "631994f8-fd2e-48bf-af8f-e86320e17d94";
constexpr char kTemperature[] = "b5634099-97c4-4f37-adf3-771246e77754";
constexpr uint16_t kCapacity = 30000;
constexpr size_t kMaxSensors = 4;
constexpr size_t kMaxHistory = 48;
constexpr std::array<uint8_t, 16> kSalt = {
    0xe1, 0xda, 0x71, 0x95, 0xec, 0xed, 0x40, 0xe4,
    0xbc, 0x16, 0x55, 0x9e, 0x9d, 0xe7, 0x3b, 0xa4};

struct Sample { float voltage = 0; float temperature = 0; };
struct Metadata {
  uint32_t uptime = 0;  // Sensor seconds since power-on, NOT Unix time.
  uint8_t periodMinutes = 0;
  uint16_t cursor = 0;  // Next write position; latest sample is cursor - 1 modulo 30000.
};
struct Range { uint16_t first = 0; uint16_t count = 0; };
struct HistoryPlan {
  std::array<Range, 2> ranges{};
  uint16_t count = 0;
  uint16_t newCount = 0;
  uint32_t omitted = 0;
  bool reset = false;
};

bool normalizeCode(const std::string& input, std::string& output);
bool normalizeAddress(const std::string& input, std::string& output);
std::array<uint8_t, 32> unlockInput(const std::string& normalizedCode);
bool parseMetadata(const uint8_t* clock, size_t clockSize,
                   const uint8_t* period, size_t periodSize,
                   const uint8_t* cursor, size_t cursorSize, Metadata& output);
bool decodeSamples(const uint8_t* bytes, size_t length, Sample* output,
                   size_t capacity, size_t& count);
bool decodeLiveVoltage(const uint8_t* bytes, size_t length, float& voltage);
bool decodeTemperature(const uint8_t* bytes, size_t length, float& temperature);
bool hasHistory(const Metadata& metadata);
uint32_t latestAgeSeconds(const Metadata& metadata);
HistoryPlan planHistory(const Metadata& current, const Metadata* previous);
std::array<uint8_t, 5> rangePayload(const Range& range);
uint64_t sampleEpoch(uint64_t readEpoch, const Metadata& metadata,
                     size_t count, size_t index);
bool elapsed(uint32_t now, uint32_t then, uint32_t interval);
}  // namespace ctek
