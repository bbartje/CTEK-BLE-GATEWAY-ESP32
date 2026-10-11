#include "admin_access.h"
#include <cassert>
#include <iostream>
#include <string>

int main() {
  using namespace ctek;
  // Empty/corrupt storage and the public default must never unlock operation.
  for (const std::string& password : {std::string(), std::string("short"),
                                    std::string(64, 'a'), std::string("Spatie erin12"),
                                    std::string(kInitialAdminPassword)})
    assert(passwordChangeRequired(password));
  assert(validAdminPassword(kInitialAdminPassword));  // Valid WPA2 bootstrap key.
  assert(!validReplacementPassword(kInitialAdminPassword));
  assert(!passwordChangeRequired("EigenNieuw123!"));
  assert(!passwordChangeRequired("AbCdEfGh23456789"));  // Older unique passwords remain valid.
  assert(validReplacementPassword(std::string(12, 'a')));
  assert(validReplacementPassword(std::string(63, 'a')));
  assert(!validReplacementPassword(std::string(11, 'a')));
  assert(!validReplacementPassword(std::string(64, 'a')));
  assert(!validReplacementPassword("Wachtwoordé123"));
  assert(!validReplacementPassword(std::string("abcdefghijkl\0x", 14)));
  assert(!validReplacementPassword("abcdefghijkl\n"));
  assert(!validReplacementPassword("abcdefghijkl "));
  assert(validReplacementPassword("Andere<Code>&\"'123"));

  assert(checkInitialPassword(kInitialAdminPassword, kInitialAdminPassword) == PasswordProblem::UnchangedDefault);
  assert(checkInitialPassword("EigenNieuw123!", "AnderNieuw123!") == PasswordProblem::Mismatch);
  assert(checkInitialPassword("short", "short") == PasswordProblem::Invalid);
  assert(checkInitialPassword("EigenNieuw123!", "") == PasswordProblem::Mismatch);
  assert(checkInitialPassword("EigenNieuw123!", "EigenNieuw123!") == PasswordProblem::None);

  // Both the normal route guard and multipart OTA use this production policy.
  for (bool changeRequired : {false, true}) {
    for (bool passwordPage : {false, true})
      assert(webAccess(false, changeRequired, passwordPage) == WebAccess::LoginRequired);
  }
  assert(webAccess(true, true) == WebAccess::PasswordChangeRequired);
  assert(webAccess(true, true, true) == WebAccess::Allowed);
  assert(webAccess(true, false) == WebAccess::Allowed);
  assert(webAccess(true, false, true) == WebAccess::Allowed);
  std::cout << "Beheertests geslaagd: verplicht vervangen startwachtwoord, invoergrenzen, herhaling en toegangsbeleid.\n";
}
