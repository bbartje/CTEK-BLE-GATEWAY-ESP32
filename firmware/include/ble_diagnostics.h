#pragma once
#include <cstdint>

namespace ctek {
struct DisconnectTrace {
  bool seen = false, unlockStarted = false;
  int reason = 0;
  uint32_t sessionMs = 0, sinceUnlockMs = 0;
  const char* phase = "verbinden";
};

// One instance per BLE client. The adapter locks access across worker/callback.
// Phase labels must be static strings; no sensor data or credentials belong here.
class BleDiagnostics {
 public:
  explicit BleDiagnostics(uint32_t startedMs) : startedMs_(startedMs) {}
  void setPhase(const char* phase) { phase_ = phase; }
  void startUnlock(uint32_t now) {
    phase_ = "ontgrendelwrite";
    unlockStarted_ = true;
    unlockMs_ = now;
  }
  void disconnected(int reason, uint32_t now) {
    if (cleanup_ || trace_.seen) return;
    trace_ = {true, unlockStarted_, reason, uint32_t(now - startedMs_),
              unlockStarted_ ? uint32_t(now - unlockMs_) : 0, phase_};
  }
  DisconnectTrace beforeCleanup() {
    // Intentional client deletion must not look like a sensor failure.
    cleanup_ = true;
    return trace_;
  }
 private:
  uint32_t startedMs_, unlockMs_ = 0;
  bool unlockStarted_ = false, cleanup_ = false;
  const char* phase_ = "verbinden";
  DisconnectTrace trace_{};
};
}  // namespace ctek
