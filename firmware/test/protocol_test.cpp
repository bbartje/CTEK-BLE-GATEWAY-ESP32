#include "ctek_protocol.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

int main() {
  using namespace ctek;
  std::string normalized;
  assert(normalizeCode(" abcd efgh ijkl mnop\n", normalized) && normalized == "ABCDEFGHIJKLMNOP");
  assert(normalizeCode("12345678901", normalized) && normalized == "4014912345678901");
  assert(!normalizeCode("123", normalized));
  assert(!normalizeCode("123456789012345\xc3\xa9", normalized));
  assert(!normalizeCode(std::string(65, 'X'), normalized));
  assert(normalizeAddress("2c:fc:e4:01:02:03", normalized) && normalized == "2C:FC:E4:01:02:03");
  assert(!normalizeAddress("2C:FC:E4:01:02:ZZ", normalized));
  assert(!normalizeAddress("2C:FC:E4:01", normalized));
  auto auth = unlockInput("ABCDEFGHIJKLMNOP");
  assert(auth[0] == 0xe1 && auth[15] == 0xa4 && auth[16] == 'A' && auth[31] == 'P');

  Sample samples[3]; size_t count;
  const uint8_t data[] = {0x00, 0x64, 0x29, 0x00, 0x60, 0xeb, 0x00, 0x68, 0x80};
  assert(decodeSamples(data, sizeof(data), samples, 3, count) && count == 3);
  assert(samples[0].voltage == 12.5f && samples[0].temperature == 20.5f);
  assert(samples[1].voltage == 12 && samples[1].temperature == -10.5f);
  assert(samples[2].voltage == 13 && samples[2].temperature == -64);
  assert(!decodeSamples(data, 4, samples, 3, count) && count == 0);
  assert(!decodeSamples(data, 9, samples, 2, count));
  assert(!decodeSamples(nullptr, 3, samples, 3, count));
  float voltage, temperature;
  assert(decodeLiveVoltage(data, 2, voltage) && voltage == 12.5f);
  assert(!decodeLiveVoltage(data, 3, voltage));
  assert(decodeTemperature(data + 5, 1, temperature) && temperature == -10.5f);

  const uint8_t clock[] = {0xc0, 0x03, 0x00, 0x00}, period[] = {5}, cursor[] = {3, 0};
  Metadata meta;
  assert(parseMetadata(clock, 4, period, 1, cursor, 2, meta));
  assert(meta.uptime == 960 && meta.cursor == 3 && latestAgeSeconds(meta) == 60);
  assert(!parseMetadata(clock, 3, period, 1, cursor, 2, meta));
  const uint8_t zero[] = {0}, invalidCursor[] = {0x30, 0x75};
  assert(!parseMetadata(clock, 4, zero, 1, cursor, 2, meta));
  assert(!parseMetadata(clock, 4, period, 1, invalidCursor, 2, meta));
  auto plan = planHistory(meta, nullptr);
  assert(plan.count == 3 && plan.newCount == 3 && plan.ranges[0].first == 0);
  auto bytes = rangePayload(plan.ranges[0]);
  assert((bytes == std::array<uint8_t,5>{{3,0,0,3,0}}));
  assert(sampleEpoch(2000000000, meta, 3, 0) == 1999999340);
  assert(sampleEpoch(2000000000, meta, 3, 2) == 1999999940);
  assert(sampleEpoch(0, meta, 3, 2) == 0);
  assert(sampleEpoch(2, meta, 3, 2) == 0);
  assert(sampleEpoch(2000000000, meta, 3, 3) == 0);

  Metadata empty{200, 5, 0};
  assert(!hasHistory(empty) && planHistory(empty, nullptr).count == 0);
  Metadata wrapped{9000060, 5, 0};
  assert(hasHistory(wrapped));
  plan = planHistory(wrapped, nullptr);
  assert(plan.count == 12 && plan.ranges[0].first == 29988 && plan.ranges[0].count == 12);
  Metadata beforeWrap{8999460, 5, 29998}, afterWrap{9000360, 5, 1};
  plan = planHistory(afterWrap, &beforeWrap);
  assert(plan.count == 3 && plan.newCount == 3 && !plan.reset);
  assert(plan.ranges[0].first == 29998 && plan.ranges[0].count == 2);
  assert(plan.ranges[1].first == 0 && plan.ranges[1].count == 1);
  assert((rangePayload(plan.ranges[0]) == std::array<uint8_t,5>{{3,0x2e,0x75,2,0}}));
  plan = planHistory(meta, &meta);
  assert(plan.count == 1 && plan.newCount == 0);
  Metadata advanced{30960, 5, 103};
  plan = planHistory(advanced, &meta);
  assert(plan.count == 48 && plan.omitted == 52 && !plan.reset);
  plan = planHistory(meta, &advanced);
  assert(plan.reset && plan.count == 3);
  Metadata differentPeriod{960, 1, 16};
  assert(planHistory(differentPeriod, &meta).reset);
  Metadata corruptProgress{1260, 5, 200};
  assert(planHistory(corruptProgress, &meta).reset);
  uint32_t nearWrap = std::numeric_limits<uint32_t>::max() - 99;
  assert(elapsed(100, nearWrap, 200));
  assert(!elapsed(99, nearWrap, 200));
  std::cout << "Protocoltests geslaagd: authenticatie-invoer, parsing, historie, klok, grensgevallen.\n";
}
