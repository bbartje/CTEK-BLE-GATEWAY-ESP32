#include "firmware_image.h"
#include <array>
#include <cassert>
#include <iostream>

int main() {
  std::array<uint8_t, ctek::kImagePrefixSize> image{};
  image[0] = 0xe9; image[1] = 5;
  image[32] = 0x32; image[33] = 0x54; image[34] = 0xcd; image[35] = 0xab;
  memcpy(image.data() + 80, ctek::kProjectName, sizeof(ctek::kProjectName));
  assert(ctek::validOtaPrefix(image.data(), image.size()));
  assert(!ctek::validOtaPrefix(nullptr, image.size()));
  for (size_t size = 0; size < image.size(); ++size) assert(!ctek::validOtaPrefix(image.data(), size));
  image[0] = 0xff; assert(!ctek::validOtaPrefix(image.data(), image.size())); image[0] = 0xe9;
  image[12] = 5; assert(!ctek::validOtaPrefix(image.data(), image.size())); image[12] = 0;
  image[32] = 0; assert(!ctek::validOtaPrefix(image.data(), image.size())); image[32] = 0x32;
  image[80] = 'x'; assert(!ctek::validOtaPrefix(image.data(), image.size())); image[80] = 'c';
  image[1] = 17; assert(!ctek::validOtaPrefix(image.data(), image.size())); image[1] = 0;
  assert(!ctek::validOtaPrefix(image.data(), image.size()));
  std::cout << "Image-tests geslaagd: verkeerde chip, factory-bin, project en korte headers geweigerd.\n";
}
