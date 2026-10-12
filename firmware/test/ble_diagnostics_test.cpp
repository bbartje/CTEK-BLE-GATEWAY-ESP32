#include "ble_diagnostics.h"
#include <cassert>
#include <cstring>
#include <iostream>

int main() {
  using ctek::BleDiagnostics;
  BleDiagnostics early(100);
  early.setPhase("GATT-discovery");
  early.disconnected(520, 400);
  auto trace = early.beforeCleanup();
  assert(trace.seen && trace.reason == 520 && trace.sessionMs == 300);
  assert(!trace.unlockStarted && trace.sinceUnlockMs == 0);
  assert(!std::strcmp(trace.phase, "GATT-discovery"));

  BleDiagnostics waiting(100);
  waiting.startUnlock(800);
  waiting.setPhase("wachten na ontgrendelwrite");
  waiting.disconnected(531, 900);
  // A later retry or second callback cannot overwrite the actual failure.
  waiting.setPhase("klok lezen");
  waiting.disconnected(534, 3100);
  trace = waiting.beforeCleanup();
  assert(trace.seen && trace.reason == 531 && trace.sessionMs == 800);
  assert(trace.unlockStarted && trace.sinceUnlockMs == 100);
  assert(!std::strcmp(trace.phase, "wachten na ontgrendelwrite"));

  BleDiagnostics clean(100);
  assert(!clean.beforeCleanup().seen);
  clean.disconnected(534, 900);  // Asynchronous deleteClient callback.
  assert(!clean.beforeCleanup().seen);
  BleDiagnostics next(1000);  // An earlier client's cleanup never reaches this one.
  next.startUnlock(1100);
  next.disconnected(531, 1200);
  assert(next.beforeCleanup().seen);

  BleDiagnostics wrapped(UINT32_MAX - 100);
  wrapped.startUnlock(UINT32_MAX - 20);
  wrapped.disconnected(531, 30);
  trace = wrapped.beforeCleanup();
  assert(trace.sessionMs == 131 && trace.sinceUnlockMs == 51);
  assert(!std::strcmp(trace.phase, "ontgrendelwrite"));
  std::cout << "BLE-diagnosetests geslaagd: reden, fase, timing, retries, opruimen en klokoverloop.\n";
}
