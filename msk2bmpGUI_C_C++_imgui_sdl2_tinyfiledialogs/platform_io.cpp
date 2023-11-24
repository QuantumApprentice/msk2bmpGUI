#include "platform_io.h"
#include "Image2Texture.h"

#ifdef QFO2_WINDOWS
#include <Windows.h>
#include <string.h>

int ext_compare(NATIVE_STRING_TYPE* str1, NATIVE_STRING_TYPE* str2, int num_char)
{
    return _wcsnicmp(str1, str2, num_char);
}

uint64_t nano_timed()
{
    return (clock() / CLOCKS_PER_SEC);
}

#elif defined(QFO2_LINUX)
#include <strings.h>

int ext_compare(NATIVE_STRING_TYPE* str1, NATIVE_STRING_TYPE* str2, int num_char)
{
    return strncasecmp(str1, str2, num_char);
}

uint64_t nano_time()
{
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    uint64_t nanotime = (uint64_t)start.tv_sec * 1'000'000'000ULL;
    nanotime += start.tv_nsec;
    return nanotime;
    // return (start.tv_sec * 1000 + (start.tv_nsec)/(long)1000000);
}



#endif


