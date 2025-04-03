#pragma once
#include <cstdint>

#ifdef QFO2_WINDOWS
    #define NATIVE_STRING_TYPE          const wchar_t
    int io_wstrncmp(NATIVE_STRING_TYPE* str1, NATIVE_STRING_TYPE* str2, int num_char);
#elif defined(QFO2_LINUX)
    #define NATIVE_STRING_TYPE          const char
#endif

int io_strncmp(const char* str1, const char* str2, int num_char);
bool io_isdir(char* dir_path);


void* io_open_dir(const char* dir_name);
char* io_scan_dir(void* dir_stream);
bool io_close_dir(void* dir_stream);


bool io_file_exists(const char* filename);
bool io_path_check(char* file_path);
bool io_make_dir(char* dir_path);
int io_file_size(const char* filename);
bool io_backup_file(char* file_path, char* dest_path);
bool io_create_backup_dir(char* dir);
bool io_move_file(char* file_path, char* dest_dir);
char* io_load_text_file(char* full_path);
bool fallout2exe_exists(char* game_path);
