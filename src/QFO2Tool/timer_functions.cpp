#include <cstdint>
#include <time.h>
#include <stdio.h>

#ifdef QFO2_WINDOWS
#include <Windows.h>
#include <string.h>

uint64_t nano_time()
{
    LARGE_INTEGER Time;
    LARGE_INTEGER Frequency;
    QueryPerformanceCounter(&Time);
    QueryPerformanceFrequency(&Frequency);
    // this will be janky if frequency is not evenly divisible into 1'000'000'000
    return Time.QuadPart * (1'000'000'000 / Frequency.QuadPart);
}

#elif defined(QFO2_LINUX)
#include <strings.h>
#include <sys/stat.h>

//TODO: move the timing stuff to it's own translation unit/header
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

uint64_t start_timer()
{
    #ifdef QFO2_WINDOWS
        // //timing code for WINDOWS ONLY
        LARGE_INTEGER Time;
        QueryPerformanceCounter(&Time);
        return Time.QuadPart;
    #elif defined(QFO2_LINUX)
        // timing code for LINUX
        uint64_t StartingTime;
        StartingTime = nano_time();
        return StartingTime;
    #endif

}

void print_timer(uint64_t StartingTime)
{
    #ifdef QFO2_WINDOWS
        ////timing code WINDOWS ONLY
        LARGE_INTEGER Time;
        LARGE_INTEGER Frequency;
        QueryPerformanceCounter(&Time);
        QueryPerformanceFrequency(&Frequency);
        uint64_t ElapsedMicroseconds = (Time.QuadPart - StartingTime) * 1000000 / Frequency.QuadPart;
        printf("Total time elapsed: %ldμs\n", ElapsedMicroseconds);
    #elif defined(QFO2_LINUX)
        // timing code LINUX
        uint64_t EndingTime = nano_time();
        uint64_t nanoseconds_total = EndingTime - StartingTime; // = NANOSECONDS_IN_SECOND * (end.tv_sec - start.tv_sec);
        uint64_t microseconds_total = nanoseconds_total / 1000;
        printf("Total time elapsed: %ldμs\n", microseconds_total);
    #endif
}


