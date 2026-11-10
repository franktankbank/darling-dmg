#include "unichar.h"
#include "unicode_helpers.h"
#include <utf8.h>
#include <fastunicodecompare.h>
#include <iconv.h>
#include <cassert>
#include <vector>

std::string UnicharToString(uint16_t length, const unichar* string)
{
	std::u16string u16 = utf16be_to_native((const uint8_t*)string, length);
	return utf8::utf16to8(u16);
}

bool EqualNoCase(const HFSString& str1, const std::string& str2)
{
	// Convert str2 (UTF-8) to UTF-16BE in a temp buffer, then use FastUnicodeCompare
	std::u16string u16 = utf8::utf8to16(str2);
	uint16_t len2 = (uint16_t)u16.size();

	// Byte-swap str2's UTF-16 to BE in a small buffer
	std::vector<uint8_t> buf2(len2 * 2);
	native_to_utf16be(u16, buf2.data(), buf2.size());

	return FastUnicodeCompare((uint16_t*)str1.string, be(str1.length), (uint16_t*)buf2.data(), len2) == 0;
}

bool EqualCase(const HFSString& str1, const std::string& str2)
{
	std::u16string u16 = utf8::utf8to16(str2);

	uint16_t len1 = be(str1.length);
	uint16_t len2 = (uint16_t)u16.size();
	if (len1 != len2) return false;

	// Compare code unit by code unit (str1 is BE, u16 is native)
	for (uint16_t i = 0; i < len1; i++) {
		uint16_t c1 = (uint16_t)((str1.string[i*2] << 8) | str1.string[i*2+1]);
		if (c1 != u16[i]) return false;
	}
	return true;
}

uint16_t StringToUnichar(const std::string& in, unichar* out, size_t maxLength)
{
	std::u16string u16 = utf8::utf8to16(in);
	return native_to_utf16be(u16, (uint8_t*)out, maxLength * sizeof(unichar));
}
