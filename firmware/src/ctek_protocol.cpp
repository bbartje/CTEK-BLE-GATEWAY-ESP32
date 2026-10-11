#include "ctek_protocol.h"

#include <algorithm>

namespace ctek {
namespace {
bool whitespace(unsigned char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}
char uppercase(unsigned char c) { return c >= 'a' && c <= 'z' ? c - ('a' - 'A') : c; }
bool hex(unsigned char c) { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'); }
uint16_t le16(const uint8_t* p) { return uint16_t(p[0]) | (uint16_t(p[1]) << 8); }
}

bool normalizeCode(const std::string& input, std::string& output) {
  output.clear();
  if (input.size() > 64) return false;
  for (unsigned char c : input) {
    if (whitespace(c)) continue;
    // CTEK's Android UI uppercases ASCII and removes whitespace before hashing.
    if (c < 33 || c > 126) { output.clear(); return false; }
    output += uppercase(c);
  }
  if (output.size() == 11) output = "40149" + output;
  if (output.size() == 16) return true;
  output.clear();
  return false;
}

bool normalizeAddress(const std::string& input, std::string& output) {
  output.clear();
  if (input.size() != 17) return false;
  for (size_t i = 0; i < input.size(); ++i) {
    char c = uppercase(input[i]);
    if (i % 3 == 2 ? c != ':' : !hex(c)) return false;
    output += c;
  }
  return true;  // Identity is confirmed through GATT, not only an OUI prefix.
}

std::array<uint8_t, 32> unlockInput(const std::string& code) {
  std::array<uint8_t, 32> result{};
  if (code.size() != 16) return result;
  std::copy(kSalt.begin(), kSalt.end(), result.begin());
  std::copy(code.begin(), code.end(), result.begin() + 16);
  return result;
}

bool parseMetadata(const uint8_t* clock, size_t clockSize,
                   const uint8_t* period, size_t periodSize,
                   const uint8_t* cursor, size_t cursorSize, Metadata& output) {
  if (!clock || !period || !cursor || clockSize != 4 || periodSize != 1 || cursorSize != 2)
    return false;
  Metadata parsed;
  parsed.uptime = uint32_t(clock[0]) | (uint32_t(clock[1]) << 8) |
                  (uint32_t(clock[2]) << 16) | (uint32_t(clock[3]) << 24);
  parsed.periodMinutes = period[0];
  parsed.cursor = le16(cursor);
  if (!parsed.periodMinutes || parsed.cursor >= kCapacity) return false;
  output = parsed;
  return true;
}

bool decodeSamples(const uint8_t* bytes, size_t length, Sample* output,
                   size_t capacity, size_t& count) {
  count = 0;
  if (!bytes || !output || !length || length % 3 || length / 3 > capacity) return false;
  for (size_t i = 0; i < length / 3; ++i) {
    const uint8_t* p = bytes + i * 3;
    output[i].voltage = le16(p) / 2048.0f;
    int signedByte = p[2] < 128 ? p[2] : int(p[2]) - 256;
    output[i].temperature = signedByte / 2.0f;
  }
  count = length / 3;
  return true;
}

bool decodeLiveVoltage(const uint8_t* bytes, size_t length, float& voltage) {
  if (!bytes || !length || length % 2) return false;
  voltage = le16(bytes + length - 2) / 2048.0f;
  return true;
}
bool decodeTemperature(const uint8_t* bytes, size_t length, float& temperature) {
  if (!bytes || length != 1) return false;
  temperature = (bytes[0] < 128 ? int(bytes[0]) : int(bytes[0]) - 256) / 2.0f;
  return true;
}

bool hasHistory(const Metadata& m) {
  return m.periodMinutes && m.cursor < kCapacity &&
         (m.cursor != 0 || uint64_t(m.uptime) >= uint64_t(kCapacity) * m.periodMinutes * 60);
}
uint32_t latestAgeSeconds(const Metadata& m) {
  return m.periodMinutes ? m.uptime % (uint32_t(m.periodMinutes) * 60) : 0;
}

HistoryPlan planHistory(const Metadata& m, const Metadata* previous) {
  HistoryPlan plan;
  if (!hasHistory(m)) return plan;
  bool full = uint64_t(m.uptime) >= uint64_t(kCapacity) * m.periodMinutes * 60;
  uint32_t available = full ? kCapacity : m.cursor;
  uint32_t requested = std::min<uint32_t>(available, 12);
  bool continuous = previous && previous->periodMinutes == m.periodMinutes &&
                    m.uptime >= previous->uptime;
  if (continuous) {
    uint32_t period = uint32_t(m.periodMinutes) * 60;
    uint32_t advanced = m.uptime / period - previous->uptime / period;
    continuous = (previous->cursor + advanced) % kCapacity == m.cursor;
    if (continuous) {
      requested = advanced;
      plan.omitted = advanced > kMaxHistory ? advanced - kMaxHistory : 0;
      requested = std::min<uint32_t>(requested, kMaxHistory);
    }
  }
  plan.reset = previous && !continuous;
  plan.newCount = requested;
  plan.count = std::max<uint32_t>(requested, 1);  // Re-read latest for freshness, without duplicate history.
  uint16_t first = (m.cursor + kCapacity - plan.count) % kCapacity;
  uint16_t beforeWrap = std::min<uint16_t>(plan.count, kCapacity - first);
  plan.ranges[0] = {first, beforeWrap};
  plan.ranges[1] = {0, uint16_t(plan.count - beforeWrap)};
  return plan;
}

std::array<uint8_t, 5> rangePayload(const Range& range) {
  return {{3, uint8_t(range.first), uint8_t(range.first >> 8),
           uint8_t(range.count), uint8_t(range.count >> 8)}};
}
uint64_t sampleEpoch(uint64_t now, const Metadata& m, size_t count, size_t index) {
  if (!now || index >= count || !m.periodMinutes) return 0;
  uint64_t age = latestAgeSeconds(m) + uint64_t(count - index - 1) * m.periodMinutes * 60;
  return now >= age ? now - age : 0;
}
bool elapsed(uint32_t now, uint32_t then, uint32_t interval) { return uint32_t(now - then) >= interval; }
}  // namespace ctek
