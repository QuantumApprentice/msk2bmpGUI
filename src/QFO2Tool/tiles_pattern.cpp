#include <town_map_tiles.h>
#include <Edit_TILES_LST.h>
#include <tinyfiledialogs.h>

//lay patterns out entire row at a time
//must be in little endian format
//rows start from the right in the x direction
//  and y from the top?
//why are tile entries separated by 16 bytes?
//last 12 bytes (footer?) identify x/y sizes,
//  pattern entry number?
//may have to make proto file for tiles?


void TMAP_tiles_make_row(town_tile* head, user_info* usr_info)
{
    int choice = 0;
    char* tiles_lst = usr_info->game_files.TILES_LST;
    if (tiles_lst == nullptr) {
        if (strlen(usr_info->default_game_path) > 1) {
            usr_info->game_files.TILES_LST = load_tiles_lst_game(usr_info->default_game_path);
        }
        if (tiles_lst == nullptr) {
            choice = tinyfd_messageBox(
                "TILES.LST missing...",
                "Unable to find TILES.LST.\n\n"

                "TILES.LST is needed to match up\n"       //which one? proto one? or frm one?
                "the tile-name to a line number\n"
                "in TILES.LST,\n"
                "then that line number is used\n"
                "in the pattern file to indicate\n"
                "which tile is in what position.\n\n"

                "",
                "cancel", "warning", 1
            );
        }

        
    }
    town_tile* node = head;



    while (node != nullptr)
    {
        //need the line number from TILES.LST
    }
    
}

