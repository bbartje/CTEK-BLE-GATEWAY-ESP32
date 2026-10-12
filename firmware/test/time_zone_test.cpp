#include "time_zone.h"
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <utility>
#include <iostream>

int main() {
  using namespace ctek;
  const auto* amsterdam = findTimeZone("Europe/Amsterdam");
  assert(amsterdam && !strcmp(amsterdam->rule, "CET-1CEST,M3.5.0,M10.5.0/3"));
  const auto* utc = findTimeZone("UTC");
  assert(utc && !strcmp(utc->rule, "UTC0"));
  assert(findTimeZone("not/a-zone") == nullptr);
  assert(findTimeZone(nullptr) == nullptr);
  // Exercise the conversion, including both sides of the Dutch DST change.
  setenv("TZ", amsterdam->rule, 1); tzset();
  for (const auto& check : {std::pair<time_t, int>{1767225600, 1},
                           {1782864000, 2}, {1774744200, 1},
                           {1774747800, 3}}) {
    struct tm parts{};
    assert(localtime_r(&check.first, &parts));
    assert(parts.tm_hour == check.second);
  }
  setenv("TZ", utc->rule, 1); tzset();
  time_t epoch = 1782864000;
  struct tm parts{};
  assert(localtime_r(&epoch, &parts) && parts.tm_hour == 0);
  std::cout << "Tijdzonetests geslaagd: UTC, Nederlandse zomer-/wintertijd en ongeldige zones.\n";
}
