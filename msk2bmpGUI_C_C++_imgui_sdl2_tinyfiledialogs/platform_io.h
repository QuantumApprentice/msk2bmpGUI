#pragma once
#include <cstdint>

#ifdef QFO2_WINDOWS
    #define NATIVE_STRING_TYPE          const wchart_t
#elif defined(QFO2_LINUX)
    #define NATIVE_STRING_TYPE          const char
#endif

int ext_compare(NATIVE_STRING_TYPE* str1, NATIVE_STRING_TYPE* str2, int num_char);
// uint64_t nano_time();
bool io_isdir(char* dir_path);
bool io_file_exists(const char* filename);
bool io_path_check(char* file_path);
// bool io_file_check(char* file_path, char* file_name);
bool io_make_dir(char* dir_path);
int io_file_size(const char* filename);
bool io_backup_file(char* file_path);