#include "unicode_helpers.h"

std::u16string utf16be_to_native(const uint8_t* buf, uint16_t charCount) {
    std::u16string s;
    s.resize(charCount);
    for (uint16_t i = 0; i < charCount; i++)
        s[i] = (char16_t)((buf[i*2] << 8) | buf[i*2+1]);
    return s;
}

uint16_t native_to_utf16be(const std::u16string& src, uint8_t* out, size_t maxBytes) {
    uint16_t count = (uint16_t)std::min(src.size(), (size_t)(maxBytes / 2));
    for (uint16_t i = 0; i < count; i++)
    {
        out[i*2]   = (uint8_t)(src[i] >> 8);
        out[i*2+1] = (uint8_t)(src[i] & 0xFF);
    }
    return count;
}
