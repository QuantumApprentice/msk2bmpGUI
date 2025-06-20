//Thank you to @BakerStaunch!
//This is mostly a rewrite of his source code
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "DAT_extract.h"
#include "Load_Settings.h"
#include "Edit_TILES_LST.h"
#include "Proto_Files.h"

bool extract_from_DAT(const char* file_name, const char* dat_name, user_info* usr_nfo, DAT_file* dat_file, Buffer* buff);

DAT_file load_dat_file(const char* file_name, char* game_path)
{
    DAT_file dat = {0};
    if (!file_name || !game_path) {
        return dat;
    }

    char file_path[MAX_PATH];
    snprintf(file_path, MAX_PATH, "%s/%s.dat", game_path, file_name);
    int32_t dat_size = io_file_size(file_path);
    //game probably only supports up to 2gb? wonder if sfall mods this
    if (dat_size < 1 || dat_size > INT32_MAX) {
        return dat;
    }

    FILE* dat_file = fopen(file_path,"rb");
    if (!dat_file) {
        return dat;
    }

    uint8_t* dat_data = (uint8_t*)malloc(dat_size);
    int sz = fread(dat_data,1,dat_size,dat_file);
    if (sz != dat_size) {
        fclose(dat_file);
        return dat;
    }
    fclose(dat_file);

    dat.data = dat_data;
    dat.size = dat_size;
    snprintf(dat.name, MAX_PATH, "%s", file_name);

    return dat;
}

void append(char* dst, const char* src)
{
    int i = 0;
    char* ptr = NULL;
    while (dst[i] != '\0' && i < 4096) {
        i++;
        if (dst[i] == '\0') {
            i++;
        }
    }
    if (i >= 4096 - strlen(src)) {
        return;
    }
    if (i != 0) {
        ptr = &dst[i];
    } else {
        ptr = dst;
    }
    strncpy(ptr, src, strlen(src));
}

//copies buff.file_data into malloc'd char* txt
//returns pointer to char* txt
char* DAT_to_txt(Buffer* buff)
{
    char* txt = (char*)malloc(buff->file_size+1);
    memcpy(txt, buff->file_data, buff->file_size);
    return txt;
}


bool tt_file_DAT_extract(user_info* usr_nfo, STATE_export* state)
{
    memset(state->extracted, 0, 4096);

    //64mb buffer
    #define BUFF_size           (1024*1024*64)
    Buffer buff = {
        .file_size = 0,
        .file_data = (uint8_t*)malloc(BUFF_size),
    };

    DAT_file dat_file = load_dat_file("master", usr_nfo->default_game_path);

    bool success = false;
    if (usr_nfo->game_files.FRM_TILES_LST == NULL) {
        append(state->extracted, "art\\TILES\\TILES.LST");
        success = extract_from_DAT("art\\TILES\\TILES.LST", "master", usr_nfo, &dat_file, &buff);
        if (!success) {
            return false;
        }
        usr_nfo->game_files.FRM_TILES_LST = DAT_to_txt(&buff);
        _append_TMAP_tiles_LST(usr_nfo, state->handle);
    }

    if (usr_nfo->game_files.PRO_TILES_LST == NULL && state->pro == true) {
        append(state->extracted, "proto\\TILES\\TILES.LST");
        success = extract_from_DAT("proto\\TILES\\TILES.LST", "master", usr_nfo, &dat_file, &buff);
        if (!success) {
            return false;
        }
        usr_nfo->game_files.PRO_TILES_LST = DAT_to_txt(&buff);
        _append_TMAP_PRO_tiles_LST(usr_nfo, state->handle);
    }

    if (usr_nfo->game_files.PRO_TILE_MSG == NULL && state->pro == true) {
        //TODO: need to store language in state?
        append(state->extracted, "text\\english\\Game\\pro_tile.msg");
        success = extract_from_DAT("text\\english\\Game\\pro_tile.msg", "master", usr_nfo, &dat_file, &buff);
        if (!success) {
            return false;
        }
        usr_nfo->game_files.PRO_TILE_MSG = DAT_to_txt(&buff);
        //TODO: let the user choose the language
        _append_PRO_tile_MSG(usr_nfo, state->handle, state->language[0]);
    }

    free(dat_file.data);
    free(buff.file_data);
    return true;
}

bool extract_from_DAT(const char* file_name, const char* dat_name, user_info* usr_nfo, DAT_file* dat_file, Buffer* buff)
{
    if (dat_file->size < 1) {
        //TODO: log to file
        // set_popup_warning();
        printf("Error: extract_from_DAT() Unable to load %s DAT file: L%d\n", dat_name, __LINE__);
        return false;
    }

    // uint8_t* dat_ptr = dat_file.data + dat_file.size;
    int size = *(int32_t*)&dat_file->data[dat_file->size - 4];
    if (size != dat_file->size) {
        //TODO: log to file
        // set_popup_warning();
        printf("Error: extract_from_DAT() %s stored size (%d) doesn't match on-disk size (%d): L%d", dat_name, size, dat_file->size, __LINE__);
        return false;
    }
    int dir_tree_size = *(int32_t*)&dat_file->data[dat_file->size - 8];
    if (dir_tree_size < 0 || dir_tree_size > size - 8) {
        //TODO: log to file
        // set_popup_warning();
        printf("Error: extract_from_DAT() %s directory entry size (%d) out of bounds from file size (%d): L%d", dat_name, dir_tree_size, dat_file->size, __LINE__);
        return false;
    }

    uint8_t* entry_ptr = &dat_file->data[size - dir_tree_size - 8];
    int entry_count = *(int32_t*)&entry_ptr[0];
    if (entry_count < 0) {
        //TODO: log to file
        // set_popup_warning();
        printf("Error: extract_from_DAT() %s directory count (%d) out of bounds from file size (%d): L%d", dat_name, entry_count, dat_file->size, __LINE__);
        return false;
    }
    entry_ptr += 4;

    bool extract_success = false;
    uint8_t* eof_ptr = &dat_file->data[size];
    for (int idx = 0; (idx < entry_count) && (entry_ptr+4 < eof_ptr); idx++)
    {
        int path_size = *(int32_t*)&entry_ptr[0];
        if (path_size < 0 || path_size > MAX_PATH || entry_ptr + path_size + 16 >= eof_ptr) {
            //TODO: log to file
            // set_popup_warning();
            printf("Error: extract_from_DAT() Reached end of file %s after reading only %d of %d entries: L%d", dat_name, idx, entry_count, __LINE__);
            extract_success = false;
            break;
        }


        DIR_entry entry        = {0};
        entry.path_ptr         = (char*)&entry_ptr[4];
        entry.packed           = entry_ptr[4+path_size];
        entry_ptr[4+path_size] = '\0';


        // printf("%s\n", entry.path_ptr);
        if (entry.path_ptr[0] != file_name[0]) {
            entry_ptr = &entry_ptr[4+path_size+1+4+4+4];
            continue;
        }
        int cmp = io_strncasecmp(file_name, entry.path_ptr, path_size);
        if (cmp != 0) {
            entry_ptr = &entry_ptr[4+path_size+1+4+4+4];
            continue;
        }

        entry.unpack_size = *(int32_t*)&entry_ptr[4+path_size+1];
        entry.packed_size = *(int32_t*)&entry_ptr[4+path_size+1+4];
        entry.offset      = *(int32_t*)&entry_ptr[4+path_size+1+4+4];
        entry.file_ptr    = &dat_file->data[entry.offset];

        // printf("****%s\n", entry.path_ptr);

        memset(buff->file_data, 0, buff->file_size);
        // ulong temp = BUFF_size;
        buff->file_size = BUFF_size;

        // int success = uncompress(buff->file_data, &temp, entry.file_ptr, entry.packed_size);
        int success = uncompress(buff->file_data, (ulong*)&buff->file_size, entry.file_ptr, entry.packed_size);
        if (success != Z_OK) {
            printf("wtf?\n");
            extract_success = false;
            break;
        }

        //TODO: log to file
        printf("writing file to disk: %s\n", entry.path_ptr);

        char path_buff[MAX_PATH] = {0};
        snprintf(path_buff, MAX_PATH, "%s/data/%s", usr_nfo->default_game_path, entry.path_ptr);
        io_swap_slash(path_buff);

        char* path_case = io_path_check(path_buff);
        if (!io_create_path_from_file(path_case)) {
            //fail state
            printf("Unable to create missing directory: %s\n", path_case);
            extract_success = false;
            break;
        }
        if (!io_save_txt_file(path_case, (char*)buff->file_data)) {
            printf("Unable to write file: %s\n", entry.path_ptr);
            extract_success = false;
        }
        extract_success = true;
        break;
    }

    return extract_success;
}