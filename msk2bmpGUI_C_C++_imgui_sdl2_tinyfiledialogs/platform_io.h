#pragma once
#include <cstdint>

#ifdef QFO2_WINDOWS
    #define NATIVE_STRING_TYPE          const wchart_t
#elif defined(QFO2_LINUX)
    #define NATIVE_STRING_TYPE          const char
#endif

int ext_compare(NATIVE_STRING_TYPE* str1, NATIVE_STRING_TYPE* str2, int num_char);
uint64_t nano_time();