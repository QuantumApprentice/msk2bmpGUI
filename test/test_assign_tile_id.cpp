#include "test/test_header.h"


int test_assign_tile_id()
{
    town_tile head = {
        "newtile_000",
        nullptr,
        12,
        0,
        0,
        nullptr
    };
    char* tiles_lst = load_tiles_lst("/home/quantum/Programming/Qs Modding Tool/test junk/");

    assign_tile_id_f(&head, tiles_lst);  // TILES.LST loop outside


}


int test_export_pattern()
{
    pattern* out_pattern = (pattern*)calloc(1, 0x168C); //total filesize is 0x168C
    
}
