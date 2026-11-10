// fastunicodecompare.h
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int32_t FastUnicodeCompare (uint16_t str1[], uint16_t length1, uint16_t str2[], uint16_t length2);

#ifdef __cplusplus
}
#endif
