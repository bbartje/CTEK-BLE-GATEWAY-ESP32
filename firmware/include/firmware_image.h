#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace ctek {
constexpr char kProjectName[] = "ctek-ble-gateway";
constexpr size_t kImagePrefixSize = 112;
constexpr size_t kAppSlotSize = 0x1e0000;
inline bool validOtaPrefix(const uint8_t* bytes, size_t size) {
  // ESP32 image header (24), first segment header (8), app descriptor at offset 32.
  // Reject merged factory images, other ESP chips and unrelated applications.
  if (!bytes || size < kImagePrefixSize || bytes[0] != 0xe9 ||
      !bytes[1] || bytes[1] > 16 || bytes[12] || bytes[13]) return false;
  const uint8_t magic[] = {0x32, 0x54, 0xcd, 0xab};
  return !memcmp(bytes + 32, magic, 4) &&
         !memcmp(bytes + 80, kProjectName, sizeof(kProjectName));
}
}
