#pragma once

#include "Load_Settings.h"
#include "town_map_tiles.h"

struct export_state {
    char LST_path[MAX_PATH];
    bool auto_export    = false;

    bool art = false;
    bool pro = false;
    bool pat = false;

    bool export_proto   = false;
    bool export_pattern = false;

    bool chk_game_path  = false;

    bool make_FRM_LST   = false;
    bool make_PRO_LST   = false;
    bool make_PRO_MSG   = false;

    bool load_files     = false;
    bool loaded_FRM_LST = false;
    bool loaded_PRO_LST = false;
    bool loaded_PRO_MSG = false;

    bool append_FRM_LST = false;
    bool append_PRO_LST = false;
    bool append_PRO_MSG = false;

    void set_false()
    {
        auto_export = false;
        export_proto = false;
        export_pattern = false;
        chk_game_path = false;

        make_FRM_LST = false;
        make_PRO_LST = false;
        make_PRO_MSG = false;

        load_files = false;

        loaded_FRM_LST = false;
        loaded_PRO_LST = false;
        loaded_PRO_MSG = false;
        append_FRM_LST = false;
        append_PRO_LST = false;
        append_PRO_MSG = false;
    }

    void load_frm_tiles_list_failed() {
        auto_export = false;
        export_proto = false;
        export_pattern = false;
        chk_game_path = false;

        make_FRM_LST = false;
        make_PRO_LST = false;
        make_PRO_MSG = false;

        load_files = false;
        // loaded_FRM_LST   = false;
        // loaded_PRO_LST   = false;
        // loaded_PRO_MSG   = false;

        append_FRM_LST = false;
        append_PRO_LST = false;
        append_PRO_MSG = false;
    }

    void create_new_files_clicked() {
        if (loaded_FRM_LST == false) {
            make_FRM_LST = true;
        }
        else {
            append_FRM_LST = true;
        }

        if (loaded_PRO_LST == false) {
            make_PRO_LST = true;
        }
        else {
            append_PRO_LST = true;
        }

        if (loaded_PRO_MSG == false) {
            make_PRO_MSG = true;
        }
        else {
            append_PRO_MSG = true;
        }

        export_proto = true;
    }

    void append_to_frm_tiles_list_clicked() {
        auto_export = true;

        if (art) {
            load_files = true;
            append_FRM_LST = true;
        }
        if (pro) {
            export_proto = true;
            append_PRO_LST = true;
            append_PRO_MSG = true;
        }
        if (pat) {
            export_pattern = true;
        }
    }

    void append_to_pro_tiles_list_clicked() {
        auto_export = true;

        // if (art) {
        //     load_files     = true;
        //     append_FRM_LST = true;
        // }
        if (pro) {
            export_proto = true;
            append_PRO_LST = true;
            append_PRO_MSG = true;
        }
        if (pat) {
            export_pattern = true;
        }
    }

    void append_to_msg_tiles_list_clicked() {
        auto_export = true;
        // append_FRM_LST = true;
        // append_PRO_LST = true;
        append_PRO_MSG = true;
    }

    void create_new_pro_list_clicked() {
        make_PRO_LST = true;
        append_PRO_MSG = true;
        export_proto = true;
    }

    void create_new_frm_list_clicked() {
        make_FRM_LST = true;
        append_PRO_MSG = true;
        export_proto = true;
    }

    void pro_tiles_msg_failed_to_load() {
        auto_export = false;
        export_proto = false;
        export_pattern = false;
        chk_game_path = false;

        make_FRM_LST = false;
        make_PRO_LST = false;
        make_PRO_MSG = false;

        load_files = false;
        // loaded_FRM_LST   = false;
        // loaded_PRO_LST   = false;
        // loaded_PRO_MSG   = false;

        append_FRM_LST = false;
        append_PRO_LST = false;
        append_PRO_MSG = false;
    }

    void pro_tiles_lst_failed_to_load() {
        auto_export = false;
        export_proto = false;
        export_pattern = false;
        chk_game_path = false;

        make_FRM_LST = false;
        make_PRO_LST = false;
        make_PRO_MSG = false;

        load_files = false;
        // loaded_FRM_LST   = false;
        // loaded_PRO_LST   = false;
        // loaded_PRO_MSG   = false;

        append_FRM_LST = false;
        append_PRO_LST = false;
        append_PRO_MSG = false;
    }

    void add_to_fallout_2_clicked() {
        export_proto = true;

        load_files = true;

        // append_FRM_LST = true;
        append_PRO_LST = true;
        append_PRO_MSG = true;

        chk_game_path = true;
    }

    void auto_export_all_clicked() {
        if (art) {
            load_files = true;
            append_FRM_LST = true;
        }
        if (pro) {
            export_proto = true;
            append_PRO_LST = true;
            append_PRO_MSG = true;
        }
        if (pat) {
            export_pattern = true;
        }
    }

    void append_to_art_tiles_clicked() {
        chk_game_path = true;
        load_files = true;
        append_FRM_LST = true;
    }
};

struct tile_name {
    char* name_ptr;
    int length;
    tile_name* next;
};

struct tile_name_arr {
    char*    name_ptr;
    uint32_t length;
    uint32_t next;           //points to array index of next viable name
};

bool append_TMAP_tiles_LST(user_info* usr_nfo, tt_arr_handle* handle, export_state* state);
tile_name_arr* make_name_list_arr(char* new_tiles_list);

char* save_NEW_FRM_tiles_LST(tt_arr_handle* handle, char* save_path, export_state* state);
char* load_LST_file(char* game_path, char* LST_path, char* LST_file);
char* append_FRM_tiles_LST(char* tiles_lst_path, tt_arr_handle* handle, export_state* state);
bool load_FRM_tiles_LST(user_info* usr_nfo, export_state* cur_state);
char* check_FRM_LST_names(char* tiles_lst, tt_arr_handle* handle, export_state* state);
void append_FRM_tiles_POPUP(user_info* usr_nfo, tt_arr_handle* handle, export_state* state, bool auto_export);
