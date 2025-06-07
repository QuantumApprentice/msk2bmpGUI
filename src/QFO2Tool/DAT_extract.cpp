//Thank you to @BakerStaunch!
//This is mostly a rewrite of his source code
#include <stdio.h>
#include <stdlib.h>
#include "DAT_extract.h"
#include "Load_Settings.h"


DAT_file load_dat_file(char* file_name, char* game_path)
{
    DAT_file dat = {0};
    if (!file_name || !game_path) {
        return dat;
    }

    char file_path[MAX_PATH];
    snprintf(file_path, MAX_PATH, "%s%s", game_path, file_name);
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
    if (fread(dat_data,dat_size,1,dat_file) != dat_size) {
        fclose(dat_file);
        return dat;
    }
    fclose(dat_file);

    dat.data = dat_data;
    dat.size = dat_size;
    snprintf(dat.name, MAX_PATH, "%s", file_name);

    return dat;
}

void extract_from_DAT(char* file_name, char* dat_name, user_info* usr_nfo)
{
    DAT_file dat_file = load_dat_file(dat_name, usr_nfo->default_game_path);
    if (dat_file.size < 1) {
        //TODO: log to file
        // set_popup_warning();
        printf("Error: extract_from_DAT() Unable to load %s DAT file: L%d\n", dat_name, __LINE__);
        return;
    }

    // uint8_t* dat_ptr = dat_file.data + dat_file.size;
    int size = (int)dat_file.data[dat_file.size - 4];
    if (size != dat_file.size) {
        //TODO: log to file
        // set_popup_warning();
        printf("Error: extract_from_DAT() %s stored size (%d) doesn't match on-disk size (%d): L%d", dat_name, size, dat_file.size, __LINE__);
        return;
    }
    int dir_tree_size = (int)dat_file.data[dat_file.size - 8];
    if (dir_tree_size < 0 || dir_tree_size > size - 8) {
        //TODO: log to file
        // set_popup_warning();
        printf("Error: extract_from_DAT() %s directory entry size (%d) out of bounds from file size (%d): L%d", dat_name, dir_tree_size, dat_file.size, __LINE__);
        return;
    }

    uint8_t* entry_ptr = &dat_file.data[size - dir_tree_size - 8];
    int entry_count = (int)entry_ptr[0];
    if (entry_count < 0) {
        //TODO: log to file
        // set_popup_warning();
        printf("Error: extract_from_DAT() %s directory count (%d) out of bounds from file size (%d): L%d", dat_name, entry_count, dat_file.size, __LINE__);
        return;
    }
    entry_ptr += 4;

    //64mb buffer
    Buffer buff = {
        .file_size = 0,
        .file_data = (uint8_t*)malloc(1024*1024*64),
    };

    uint8_t* eof_ptr = &dat_file.data[size];
    for (int idx = 0; (idx < entry_count) && (entry_ptr+4 < eof_ptr); idx++)
    {
        int path_size = (int)entry_ptr[0];
        if (path_size < 0 || path_size > MAX_PATH || entry_ptr + path_size + 16 >= eof_ptr) {
            //TODO: log to file
            // set_popup_warning();
            printf("Error: extract_from_DAT() Reached end of file %s after reading only %d of %d entries: L%d", dat_name, idx, entry_count, __LINE__);
            return;
        }


        DIR_entry entry   = {0};
        entry.path_ptr    = (char*)&entry_ptr[4];
        entry.type        = entry_ptr[4+path_size];
        entry.unpack_size = entry_ptr[4+path_size+1];
        entry.packed_size = entry_ptr[4+path_size+1+4];
        entry.offset      = entry_ptr[4+path_size+1+4+4];
        entry.file_ptr    = &dat_file.data[entry.offset];
    }
    

}