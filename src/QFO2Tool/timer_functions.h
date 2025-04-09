#pragma once
#include <cstdint>

#ifdef QFO2_WINDOWS
    #define NATIVE_STRING_TYPE          const wchar_t
#elif defined(QFO2_LINUX)
    #define NATIVE_STRING_TYPE          const char
#endif
uint64_t nano_time();
uint64_t start_timer();
void print_timer(uint64_t StartingTime);