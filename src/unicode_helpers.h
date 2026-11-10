#ifndef UNICODE_HELPERS_H
#define UNICODE_HELPERS_H

#include <string>

std::u16string utf16be_to_native(const uint8_t* buf, uint16_t charCount);

uint16_t native_to_utf16be(const std::u16string& src, uint8_t* out, size_t maxBytes);

#endif
