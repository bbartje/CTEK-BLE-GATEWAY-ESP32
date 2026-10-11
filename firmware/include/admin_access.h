#pragma once
#include <string_view>

namespace ctek {
// Public bootstrap credential, deliberately replaced before normal operation.
inline constexpr char kInitialAdminPassword[] = "CTEK-Start123";

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
inline WebAccess webAccess(bool authenticated, bool changeRequired, bool passwordPage = false) {
  if (!authenticated) return WebAccess::LoginRequired;
  if (changeRequired && !passwordPage) return WebAccess::PasswordChangeRequired;
  return WebAccess::Allowed;
}
}  // namespace ctek
