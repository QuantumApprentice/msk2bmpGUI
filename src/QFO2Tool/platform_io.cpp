//need to look at this for windows build
//https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-replacefilea

#include "platform_io.h"
#include "Image2Texture.h"

#ifdef QFO2_WINDOWS
#include <Windows.h>
#include <string.h>
#include <direct.h>

struct Directory {
    HANDLE          hnd;
    WIN32_FIND_DATA dir_struct;
};

wchar_t* io_get_err_str(DWORD err)
{
    static wchar_t lpMsgBuf[1024];

    if (FormatMessage(
        //FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        lpMsgBuf,
        1024,
        NULL) == 0) {
        int wtf = GetLastError();
        MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
        ExitProcess(err);
    }

    return lpMsgBuf;
}

char* io_wchar_utf8(NATIVE_STRING_TYPE* src)
{
    int len = wcslen(src);
    static char buff[MAX_PATH];
    LPBOOL lpUsedDefaultChar = false;
    int count = WideCharToMultiByte(
        CP_UTF8,
        0,
        src,
        -1,                 //  -1 if string is null-terminated
        buff,
        MAX_PATH,
        NULL,
        lpUsedDefaultChar
    );

    if (count == 0) {
        int err = GetLastError();
        printf("Error: io_wchar_utf8() : %d\n%S\n", err, io_get_err_str(err));
        return NULL;
    }

    if (lpUsedDefaultChar) {
        printf("io_wchar_char() : Used a default char in conversion.\n");
    }

    return buff;
}

wchar_t* io_utf8_wchar(const char* src)
{
    int len = strlen(src);
    static wchar_t buff[MAX_PATH];
    int count = MultiByteToWideChar(
        CP_UTF8,
        0,
        src,
        -1,                 // -1, the function processes the entire input string, including the terminating null character
        buff,
        MAX_PATH
    );

    if (count == 0) {
        int err = GetLastError();
        printf("Error: io_wchar_utf8() : %d\n%S\n", err, io_get_err_str(err));
        return NULL;
    }

    return buff;
}

char* io_get_cwd()
{
    return _getcwd(NULL, 0);
}


// return 0 == match, <0 == less than match, >0 == greater than match
bool io_wstrncmp(NATIVE_STRING_TYPE* str1, NATIVE_STRING_TYPE* str2, int num_char)
{
    return _wcsnicmp(str1, str2, num_char);
}

//returns -1/0/1
// int io_strncasecmp(std::filesystem::path src, void* iter_src, size_t size)
//int io_strncasecmp(const char* str1, const char* str2, int num_char)
int io_strncasecmp(NATIVE_STRING_TYPE* wstr1, NATIVE_STRING_TYPE* wstr2, int num_char)
{
    return (CompareStringEx(
        LOCALE_NAME_USER_DEFAULT,
        LINGUISTIC_IGNORECASE,
        wstr1, -1,
        wstr2,
        -1, NULL, NULL, NULL) - 2
        //-2 to convert windows bs string
        //  to match the standard  ( -1/0/1 )
    );
}

// return 0 == match, <0 == less than match, >0 == greater than match
int io_strncmp(const char* str1, const char* str2, int num_char)
{
    return strncmp(str1, str2, num_char);
}

// match this return to the linux version
bool io_isdir(char* dir_path)
{
    struct __stat64 stat_info;
    int error = _wstat64(io_utf8_wchar(dir_path), &stat_info);
    if (error == 0 && (stat_info.st_mode & _S_IFDIR) != 0) {
        /* dir_path exists and is a directory */
        return true;
    }
    return false;
    //return (stat_info.st_mode & S_IFDIR);
}



//returns false if dir_name not found?
void* io_open_dir(char* dir_name)
{
    if (strlen(dir_name) + 3 > MAX_PATH) {
        //TODO: log to file
        printf("Directory name too long: %s\n", dir_name);
        return NULL;
    }

    char search_buff[MAX_PATH];
    snprintf(search_buff, MAX_PATH, "%s\\*", dir_name);

    wchar_t* str = io_utf8_wchar(search_buff);
    Directory* dir = (Directory*)calloc(1, sizeof(Directory));
    dir->hnd = FindFirstFile(str, &dir->dir_struct);
    if (dir->hnd == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            printf("File not found\n");
            return NULL;
        }

        //TODO: log to file
        printf("Error: io_open_dir(): %d\n%S\n", err, io_get_err_str(err));
        return NULL;
    }

    return (void*)dir;
}

char* io_scan_dir(void* dir_stream)
{
    Directory*       ptr = (Directory*)dir_stream;
    HANDLE           hnd = ptr->hnd;
    WIN32_FIND_DATA data = ptr->dir_struct;

    int success = FindNextFile(
        hnd,
        &data
    );
    if (success == 0) {
        //TODO: log to file
        DWORD err = GetLastError();
        if (err != ERROR_NO_MORE_FILES) {
            printf("Error: io_scan_dir() : %d\n%S\n", err, io_get_err_str(err));
        }
        return NULL;
    }

    char* file_name = io_wchar_utf8(data.cFileName);

    return file_name;
}

bool io_close_dir(void* dir_stream)
{
    Directory* ptr = (Directory*)dir_stream;
    HANDLE     hnd = ptr->hnd;

    int err = FindClose(hnd);
    if (err == 0) {
        //TODO: log to file
        DWORD err = GetLastError();
        printf("Error: io_close_dir() : %d\n%S\n", err, io_get_err_str(err));
        return false;
    }

    return true;
}

//returns correct path for case insensitive input
//  returns original path if no matching path is found
//  this is currently designed to need a create_path_()
//  function call following a call to this
char* io_path_check(char* file_name)
{
    return file_name;
}

int io_file_size(const char* filename)
{
    struct stat stat_info;
    int error = stat(filename, &stat_info);
    if (error) {
        //TODO: log to file
        DWORD err = GetLastError();
        printf("Error: io_close_dir() : %d\n%S\n", err, io_get_err_str(err));
        return 0;
    }
    return (stat_info.st_size);
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
        //TODO: log to file
        if (errno != ENOENT) {
            printf("Error: io_file_exists() : %d\n", error);
        }
        return false;
    }
    return (stat_info.st_mode & S_IFREG);
}

//makes a directory from the path provided
//recursively makes leading directories if they don't exist
//returns true on success or directory exists, false on error
//TODO: make this wchar_t compatible?
bool io_make_dir(char* dir_path)
{
    int error;
    error = _mkdir(dir_path);
    if (error == 0) {
        return true;
    }
    else {
        if (errno == ENOENT) {
        char* ptr = strrchr(dir_path, '/');
        *ptr = '\0';
        if (io_make_dir(dir_path)) {
            *ptr = '/';
            return io_make_dir(dir_path);
            }
        }
        else {
            printf("You may ask yourself, how did I get here?\n");
        }
    }
    //TODO: log to file
    printf("Error making directory, errno:\t%d:\t%s\n", errno, strerror(errno));
    return false;
}


#elif defined(QFO2_LINUX)
#include <strings.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

char* io_get_cwd()
{
    return getcwd(NULL, 0);
}

int io_strncasecmp(NATIVE_STRING_TYPE* str1, NATIVE_STRING_TYPE* str2, int num_char)
{
    return strncasecmp(str1, str2, num_char);
}

// return 0 == match, <0 == less than match, >0 == greater than match
int io_strncmp(NATIVE_STRING_TYPE* str1, NATIVE_STRING_TYPE* str2, int num_char)
{
    return strncasecmp(str1, str2, num_char);
}

//returns true if path exists and is a directory
//false otherwise, or if error occurs
bool io_isdir(char* dir_path)
{
    struct stat stat_info;
    int error = stat(dir_path, &stat_info);
    if (error) {
        //TODO: log to file
        if (errno != ENOENT) {
            printf("Error checking directory? %s\n", strerror(errno));
        }
        return false;
    }
    return (stat_info.st_mode & S_IFDIR);
}

//returns DIR* stream for linux
void* io_open_dir(char* dir_name)
{
    DIR* dir_stream = opendir(dir_name);

    return dir_stream;
}

//TODO: handle files vs folders
//      with a flag input of some sort
//returns folders/files inside dir_stream location
char* io_scan_dir(void* dir_stream)
{
    errno = 0;
    struct dirent* iterator;
    iterator = readdir((DIR*)dir_stream);
    if (iterator == NULL) {
        if (errno != 0) {
            //TODO: log to file
            printf("Error: io_scan_dir() : errno: %d\n", errno);
        }
        return NULL;
    }

    char* file_name = iterator->d_name;

    return file_name;
}

bool io_close_dir(void* dir_stream)
{
    int error = closedir((DIR*)dir_stream);
    if (error) {
        //TODO: log to file
        printf("Error: io_close_dir() %s\n", strerror(errno));
        return false;
    }
    return true;
}

//returns correct path for case insensitive input
//  returns original path if no matching path is found
//  this is currently designed to need a create_path_()
//  function call following a call to this
char* io_path_check(char* file_name)
{
    static char full_path[MAX_PATH];
    strncpy(full_path, file_name, MAX_PATH);

    bool isdir = 0;
    char* curr = NULL;
    char* last = NULL;
    while (!isdir) {
        curr = strrchr(full_path, '/');
        if (!curr) {
            return NULL;
        }
        if (last) {
            last[0] = '/';
        }
        last = curr;
        curr[0] = '\0';
        isdir = io_isdir(full_path);
    }

    while(curr[1] != '\0') {
        DIR* stream = (DIR*)io_open_dir(full_path);

        int match  = -1;
        char* dir  = NULL;
        char* next = strchr(curr+1, '/');
        if (!next) {
            curr[0] = '/';
            break;
        }
        next[0] = '\0';
        while (match) {
            dir = io_scan_dir(stream);
            //no match found for this folder
            //pass full path back so new folders can be made
            if (dir == NULL) {
                curr[0] = '/';
                next[0] = '/';
                return full_path;
            }
            match = io_strncasecmp(curr+1, dir, MAX_PATH);
        }
        snprintf(curr, MAX_PATH-strlen(full_path), "/%s", dir);
        curr += strlen(dir)+1;

        io_close_dir(stream);
    }

    return full_path;
}

//another way to check if directory exists?
// #include <stdbool.h>  //bool type ?
//returns true if the file exists, false otherwise
bool io_file_exists(const char* filename)
{
    struct stat stat_info;
    int error = stat(filename, &stat_info);
    if (error) {
        if (errno != ENOENT) {
            //TODO: log to file
            printf("Error io_file_exists(): %s\n", strerror(errno));
        }
        return false;
    }
    return (stat_info.st_mode & S_IFREG);
}

int io_file_size(const char* filename)
{
    struct stat stat_info;
    int error = stat(filename, &stat_info);
    if (error) {
        //TODO: log to file
        printf("Error io_file_size() %s\n", strerror(errno));
        return 0;
    }
    return (stat_info.st_size);
}

//makes a directory from the path provided
//recursively makes leading directories if they don't exist
//returns true on success or directory exists, false on error
bool io_make_dir(char* dir_path)
{
    int error;
    error = mkdir(dir_path, (S_IRWXU | S_IRWXG | S_IRWXO));
    if (error == 0 || errno == EEXIST) {
        //successfully created directory (or it was already there)
        return true;
    }
    if (errno == ENOENT) {  // No such file or directory
        char* ptr = strrchr(dir_path, PLATFORM_SLASH);
        *ptr = '\0';
        if (io_make_dir(dir_path)) {
            *ptr = PLATFORM_SLASH;
            return io_make_dir(dir_path);
        }
    }
    //TODO: log to file
    printf("Error making directory, errno:\t%d:\t%s\n", errno, strerror(errno));
    return false;
}


#endif


//create folder paths if they don't exist
bool io_create_path_from_file(char* file_path)
{
    // char* ptr = strrchr(file_path, PLATFORM_SLASH);
    //TODO: figure out how to get PLATFORM_SLASH back in here for windows?
    char* ptr = strrchr(file_path, '/');
    char back = ptr[0];
    ptr[0] = '\0';
    if (!io_isdir(file_path)) {
        bool success = io_make_dir(file_path);
        if (!success) {
            return false;
        }
    }
    ptr[0] = back;

    return true;
}

//renames a file from file_path
//to dest_path as a backup file
//appends date&time in this format
//      _yyyymmdd_hhmmss
//appends same file extension as source
//TODO: need windows version? rename() is linux specific?
bool io_backup_file(char* src_path, char* dst_path)
{
    if (dst_path == nullptr) {
        dst_path = src_path;
    }
    char* extension = strrchr(src_path, '\0');  //get end of string position?
    char time_buff[32];
    char rename_buff[MAX_PATH];
    time_t t = time(NULL);
    tm* tp = localtime(&t);
    strftime(time_buff, 32, "_%Y%m%d_%H%M%S", tp);
    snprintf(rename_buff, MAX_PATH, "%s%s%s", dst_path, time_buff, extension-4);

    int error = rename(src_path, rename_buff);
    if (error != 0) {
        //TODO: log to file
        perror("Error backing up file: ");
        // perror("Error backing up file: errno:%s\n", strerror(error));
        return false;
    }
    return true;
}

bool io_move_file(char* file_path, char* dest_dir)
{
    char* file_name = strrchr(file_path, '/')+1;
    char rename_buff[MAX_PATH];
    snprintf(rename_buff, MAX_PATH, "%s%s", dest_dir, file_name);

    int error = rename(file_path, rename_buff);
    if (error != 0) {
        //TODO: log to file
        perror("Error moving file: ");
        return false;
    }
    return true;
}

bool io_create_backup_dir(char* dir)
{
    char dest_path[MAX_PATH];
    strncpy(dest_path, dir, MAX_PATH);

    char time_buff[32];
    time_t t = time(NULL);
    tm* tp = localtime(&t);
    strftime(time_buff, 32, "_%Y%m%d_%H%M%S", tp);
    snprintf(dir, MAX_PATH, "%s/backup%s", dest_path, time_buff);

    return io_make_dir(dest_path);
}

//loads a text file into a buffer
//returns the buffer
char* io_load_txt_file(char* full_path)
{
    if (io_file_exists(full_path) == false) {
        return nullptr;
    }

    //read the entire file into memory and return ptr
    int file_size = io_file_size(full_path);
    char* text_file_buff = (char*)malloc(file_size+1);

    FILE* tiles_lst = fopen(full_path, "rb");
    fread(text_file_buff, file_size, 1, tiles_lst);
    fclose(tiles_lst);
    //TODO: should I put error checking in
    //      to verify the entire file read in correctly?

    text_file_buff[file_size] = '\0';
    return text_file_buff;
}

bool io_save_txt_file(char* path, char* txt)
{
    if (path == nullptr) {return false;}
    if (txt[0] == '\0')  {return false;}

    FILE* txt_file = fopen(path, "wb");
    if (txt_file == NULL) {
        //TODO: log to file
        printf("Error: io_save_txt_file() : unable to open file to write to: %d\n", __LINE__);
        return false;
    }
    fwrite(txt, strlen(txt), 1, txt_file);
    fclose(txt_file);

    return true;
}

bool fallout2exe_exists(const char* game_path)
{
    char temp[MAX_PATH];
    snprintf(temp, MAX_PATH, "%s/fallout2.exe", game_path);
    return io_file_exists(temp);
}