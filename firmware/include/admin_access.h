#pragma once
#include <cstdint>
#include <string_view>

namespace ctek {
// Public bootstrap credential, deliberately replaced before normal operation.
inline constexpr char kInitialAdminPassword[] = "CTEK-Start123";
inline constexpr uint32_t kRecoveryCredentialWindowMs = 120000;

inline bool recoveryCredentialWindowOpen(uint32_t nowMs, uint32_t startedAtMs) {
  return uint32_t(nowMs - startedAtMs) < kRecoveryCredentialWindowMs;
}

inline bool recoveryLoginAllowed(bool setupPending, bool windowOpen,
                                 bool setupInterface, bool setupPeer) {
  return !setupPending && windowOpen && setupInterface && setupPeer;
}

inline bool validAdminPassword(std::string_view password) {
  if (password.size() < 12 || password.size() > 63) return false;
  for (unsigned char character : password) if (character < 33 || character > 126) return false;
  return true;
}

inline bool passwordChangeRequired(std::string_view savedPassword) {
  return !validAdminPassword(savedPassword) || savedPassword == kInitialAdminPassword;
}

inline bool validReplacementPassword(std::string_view password) {
  return validAdminPassword(password) && password != kInitialAdminPassword;
}

enum class PasswordProblem { None, Invalid, UnchangedDefault, Mismatch };
inline PasswordProblem checkInitialPassword(std::string_view password, std::string_view confirmation) {
  if (!validAdminPassword(password)) return PasswordProblem::Invalid;
  if (password == kInitialAdminPassword) return PasswordProblem::UnchangedDefault;
  if (password != confirmation) return PasswordProblem::Mismatch;
  return PasswordProblem::None;
}

enum class WebAccess { LoginRequired, PasswordChangeRequired, Allowed };
inline WebAccess webAccess(bool authenticated, bool changeRequired, bool passwordPage = false,
                          bool recoveryAuthenticated = false) {
  if (!authenticated) {
    if (recoveryAuthenticated)
      return passwordPage ? WebAccess::Allowed : WebAccess::PasswordChangeRequired;
    return WebAccess::LoginRequired;
  }
  if (changeRequired && !passwordPage) return WebAccess::PasswordChangeRequired;
  return WebAccess::Allowed;
}
}  // namespace ctek
