//Town-map tiles apparently need matching proto (.pro) files
//in order to be added to the mapper correctly.
//The mapper has a built-in function that can be used to add
//proto files, but configuration and setting up directory
//structure to match what the mapper needs is a PITA.

//Need to add entries to:
//data/proto/tiles/TILES.LST            //proto version
//data/Text/english/Game/pro_tile.msg   //alternates for other languages?


//Mapper produces interim text files to produce proto files:
/*      //example//
pid: 67111980 00003116      //proto ID number?  //line number in TILES.LST art file //check hex trick used to produce proto id from line number
name: abc                   //name (not required?)
message_num: 311600         //key/value pair in pro_tile.msg (Fallout 2/data/text/english/game/pro/proto_tile.msg)
fid: 67111979 test013       //fid == art id? has name of file
flags: 0                    //unkown?
flags_ext: 0                //unkown?
material: Glass             //material type == enum
*/

//tile proto files have: https://falloutmods.fandom.com/wiki/PRO_File_Format#Tiles
/*
0x0000  4bytes  ObjectType & ObjectID
0x0004  4bytes  TextID
0x0008  4bytes  FrmType & FrmID
0x000C  4bytes  Light radius
0x0010  4bytes  Light intensity
0x0014  4bytes  Flags
0x0018  4bytes  MaterialID
*/

#include <stdint.h>
#include <stdio.h>
#include <tinyfiledialogs.h>

#include "Proto_Files.h"
#include "Load_Settings.h"
#include "B_Endian.h"

enum material {
    Glass   = 0,
    Metal   = 1,
    Plastic = 2,
    Wood    = 3,
    Dirt    = 4,
    Stone   = 5,
    Cement  = 6,
    Leather = 7
};


void export_tile_proto_start(user_info* usr_nfo, town_tile* head)
{
    proto_info info;
    //input name
    ImGui::Text(
        "In order to get new tiles to appear in the mapper\n"
        "(and thus in the game), each tile must have a proto(.pro)\n"
        "file made, and an entry for each tile appended to\n\n"
        "   Fallout 2/data/proto/tiles/TILES.LST\n"
        "   Fallout 2/data/art/tiles/TILES.LST.\n\n"
        "In addition, entries? can? be made in\n\n"
        "   Fallout 2/data/Text/english/Game/pro_tile.msg\n\n"
        "to give the tile a name and description in the\n"
        "Fallout 2 mapper (Mapper2.exe).\n\n"
        "These are Optional."
        );
    static char buf1[32]  = ""; ImGui::InputText("Name",                 buf1, 32);
    static char buf2[512] = ""; ImGui::InputTextMultiline("Description", buf2, 512);
    //TODO: need to write name out to Fallout 2/data/Text/english/Game/pro_tile.msg
    info.name        = buf1;
    info.description = buf2;
    info.material_id = get_material_id();

    if (ImGui::Button("Export...")) {
        //TODO: place a warning here, this needs town_tile*head to work
        town_tile* node = head;
        while (node != nullptr)
        {
            export_tile_proto(usr_nfo->default_save_path, node, &info);
            node = node->next;
        }
    }

    if (ImGui::Button("Close")) {
        ImGui::CloseCurrentPopup();
    }
}

//export individual tile proto to save_path
void export_tile_proto(char* save_path, town_tile* tile, proto_info* info)
{
    if (tile == nullptr) {
        return;
    }

    char path_buff[MAX_PATH];
    char temp = tile->name_ptr[tile->length];
    tile->name_ptr[tile->length] = '\0';

    tile_proto proto;
    proto.ObjectID        = tile->tile_id | 0x4000000;
    proto.TextID          = tile->tile_id * 100;
    //FrmID is the line number (starting from 0?) in art/tiles/TILES.LST
    proto.FrmID           = (tile->tile_id - 1) | 0x4000000;    // -1 for off by 1 error?
    //TODO: test if these 3 have effect on tiles
    proto.Light_Radius    = 0;
    proto.Light_Intensity = 0;
    proto.Flags           = 0xFFFFFFFF;     //this is what the mapper uses on tiles, not sure why yet
    //end TODO
    proto.MaterialID      = info->material_id;

    B_Endian::swap_32(proto.ObjectID);
    B_Endian::swap_32(proto.TextID);
    B_Endian::swap_32(proto.MaterialID);


    snprintf(path_buff, MAX_PATH, "%s/%s.pro", save_path, tile->name_ptr);
    tile->name_ptr[tile->length] = temp;

    FILE* tile_pro = fopen(path_buff, "wb");
    fwrite(&proto, sizeof(tile_proto), 1, tile_pro);
    fclose(tile_pro);

}

//dropdown menu picking type of material
//to set the proto as
//     0: Glass
//     1: Metal
//     2: Plastic
//     3: Wood
//     4: Dirt
//     5: Stone
//     6: Cement
//     7: Leather
int get_material_id()
{
    //input material type
    static int material_id = 0;
    const char* names[] = { "Glass", "Metal", "Plastic", "Wood", "Dirt", "Stone", "Cement", "Leather" };
    // Simple selection popup (if you want to show the current selection inside the Button itself,
    // you may want to build a string using the "###" operator to preserve a constant ID with a variable label)

    if (ImGui::Button("Select Material Type...")) {
        ImGui::OpenPopup("material_select");
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(material_id == 0 ? "Glass" : names[material_id]);
    if (ImGui::BeginPopup("material_select"))
    {
        ImGui::SeparatorText("Material");
        for (int i = 0; i < IM_ARRAYSIZE(names); i++) {
            if (ImGui::Selectable(names[i])) {
                material_id = i;
            }
        }

        ImGui::EndPopup();
    }
    return material_id;
}