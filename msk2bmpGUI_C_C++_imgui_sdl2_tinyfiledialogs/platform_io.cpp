#include "platform_io.h"
#include "Image2Texture.h"

#ifdef QFO2_WINDOWS
#include <Windows.h>
#include <string.h>

int ext_compare(NATIVE_STRING_TYPE* str1, NATIVE_STRING_TYPE* str2, int num_char)
{
    return _wcsnicmp(str1, str2, num_char);
}

uint64_t nano_time()
{
    return (clock() / CLOCKS_PER_SEC);
}

void io_isdir(char* dir_path)
{
    struct __stat64 stat_info;
    error = _wstat64(tinyfd_utf8to16(dir_path), &stat_info);
    if (error == 0 && (stat_info.st_mode & _S_IFDIR) != 0) {
        /* dir_path exists and is a directory */
        return;
    }
}

//another way to check if directory exists?
// #include <stdbool.h>  //bool type ?
//returns true if the file exists, false otherwise
//TODO: needs a windows version?
bool io_file_exists(const char* filename)
{
    struct stat stat_info;
    int error = stat(filename, &stat_info);
    if (error) {
        return false;
    }
    return (stat_info.st_mode & S_IFREG);
}

//makes a directory from the path provided
//TODO: this also needs a windows version
bool make_dir(char* dir_path)
{
    int error;
    error = mkdir(dir_path, (S_IRWXU | S_IRWXG | S_IRWXO));
    if (error == 0) {
        return true;
    }
    else {
        printf("failed to created directory, error: ", errno);
        return false;
    }
}


#elif defined(QFO2_LINUX)
#include <strings.h>
#include <sys/stat.h>

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

//returns true if path exists and is a directory
//false otherwise
bool io_isdir(char* dir_path)
{
    struct stat stat_info;
    int error = stat(dir_path, &stat_info);
    if (error) {
        printf("Error checking directory? %s\n", strerror(errno));
        return false;
    }
    return (stat_info.st_mode & S_IFDIR);
}

//another way to check if directory exists?
// #include <stdbool.h>  //bool type ?
//returns true if the file exists, false otherwise
bool io_file_exists(const char* filename)
{
    struct stat stat_info;
    int error = stat(filename, &stat_info);
    if (error) {
        return false;
    }
    return (stat_info.st_mode & S_IFREG);
}

//makes a directory from the path provided
//recursively makes leading directories if they don't exist
//returns true on success or directory exists, false on error
bool io_make_dir(char* dir_path)
{
    int error;
    error = mkdir(dir_path, (S_IRWXU | S_IRWXG | S_IRWXO));
    if (error == 0) {
        return true;
    }
    else {
        if (errno == 2) {
        char* ptr = strrchr(dir_path, '/');
        *ptr = '\0';
        if (io_make_dir(dir_path)) {
            *ptr = '/';
            return io_make_dir(dir_path);
            }
        }
        else {
            printf("You may ask yourself, how did I get here?");
        }
    }
    printf("Error making directory, errno: %d, %s", errno, strerror(errno));
    return false;
}


#endif


