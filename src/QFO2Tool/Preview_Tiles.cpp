#include "imgui.h"
#include "imgui_internal.h"

#include "Preview_Tiles.h"
#include "Save_Files.h"
#include "display_FRM_OpenGL.h"
#include "Zoom_Pan.h"

#include "load_FRM_OpenGL.h"
#include "Proto_Files.h"

#include "Edit_TILES_LST.h"
#include "tiles_pattern.h"

#include "ImGui_Warning.h"
#include "DAT_extract.h"
#include "platform_dialogs.h"


// Fallout map tile size hardcoded in engine to 350x300 pixels WxH
#define MTILE_W (350)
#define MTILE_H (300)
#define MTILE_SIZE (350 * 300)

void crop_WMAP_tile(int tile_w, int tile_h, int img_w, int img_h, int scale, image_data *img_data);
void draw_red_squares(image_data *img_data, bool show_squares);
void draw_red_tiles(image_data *img_data, bool show_squares);

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

void preview_WMAP_tiles_SURFACE(variables* My_Variables, image_data* img_data)
{
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);
    shader_info* shaders = &My_Variables->shaders;

    int dir = img_data->display_orient_num;
    if (!img_data->ANM_dir) {
        ImGui::Text("No ANM_dir");
        return;
    }
    if (img_data->ANM_dir[dir].frame_data == NULL) {
        ImGui::Text("No frame_data");
        return;
    }

    animate_SURFACE_to_sub_texture(
        img_data, img_data->ANM_dir[dir].frame_data[0],
        My_Variables->CurrentTime_ms
    );

    float scale    = img_data->scale;
    int img_width  = img_data->width;
    int img_height = img_data->height;

    //TODO: rename?
    //      this takes 3 textures and draws them into 1 framebuffer
    draw_PAL_to_framebuffer(
        shaders->FO_pal,
        shaders->render_PAL_shader,
        &shaders->giant_triangle,
        img_data);

    crop_WMAP_tile(MTILE_W, MTILE_H, img_width, img_height, scale, img_data);

}

//TODO: refactor this
void crop_WMAP_tile(int tile_w, int tile_h, int img_w, int img_h, int scale, image_data *img_data)
{
    //TODO: change top_corner() for img_pos passed in from outside
    ImVec2 base_top_corner = top_corner(img_data->offset);
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    ImGuiWindow *window = ImGui::GetCurrentWindow();
    // Preview window for tiles already converted to palettized and dithered format
    ImVec2 uv_min; // = Origin;
    ImVec2 uv_max;
    int max_box_x = img_w / tile_w;
    int max_box_y = img_h / tile_h;
    int pxl_border = 3;

    for (int y = 0; y < max_box_y; y++)
    {
        for (int x = 0; x < max_box_x; x++)
        {

            uv_min.x = ((x * (float)tile_w)) / img_w;
            uv_min.y = ((y * (float)tile_h)) / img_h;

            uv_max = {(uv_min.x + ((float)tile_w / img_w)),
                      (uv_min.y + ((float)tile_h / img_h))};

            ImVec2 new_corner;
            new_corner.x = base_top_corner.x + (tile_w + pxl_border) * x * scale;
            new_corner.y = base_top_corner.y + (tile_h + pxl_border) * y * scale;

            ImVec2 new_bottom;
            new_bottom.x = new_corner.x + (tile_w * scale);
            new_bottom.y = new_corner.y + (tile_h * scale);

#pragma region render tiles
            //TODO: blit each crop to a single texture
            //      with adequate spacing between
            //      then display that texture directly in window call
            //  then use regular zoom/pan on that texture drawlist call

            // image I'm trying to pan and zoom with
            window->DrawList->AddImage(
                (ImTextureID)(uintptr_t)img_data->render_texture,
                new_corner, new_bottom,
                uv_min, uv_max,
                ImGui::GetColorU32(tint_col));
        }
    }
}


ImVec2 T_Corner =   {48,-12};
ImVec2 L_Corner = {00, 00};
ImVec2 R_Corner =       {80, 12};
ImVec2 B_Corner =   {32, 24};
void draw_TMAP_tiles(user_info* usr_nfo, image_data *img_data,
                     shader_info *shaders, GLuint tile_texture,
                     Rect* offset)
{
    float scale = img_data->scale;
    ImGuiWindow *window = ImGui::GetCurrentWindow();

    int img_w = img_data->width;
    int img_h = img_data->height;



    int col_w = (80 + 48);      // 128
    int row_h = (36 + 36 + 24); // 96
    int max_box_x = +2 * ((img_w + (col_w - 1) - offset->x) / col_w);
    int min_box_x = -3 * ((img_h + (row_h - 1) + offset->y) / row_h);
    int max_box_y = +2 * ((img_w + (col_w - 1) - offset->y) / col_w)
                    +3 * ((img_h + (row_h - 1) - offset->y) / row_h);
    int min_box_y = -3 * offset->y/ row_h - 2  * offset->x  / col_w;

    // #define pxl_per_row_x       (128)   //  ((80+80-32)    /1) tile per repeat
    // #define pxl_per_row_y        (32)   //  ((36+36+36-12) /3) tiles per repeat
    // #define pxl_per_col_x        (64)   //  ((80+80-32)    /2) tiles per repeat
    // #define pxl_per_col_y        (48)   //  ((36+36+36-12) /2) tiles per repeat

    // int max_box_x =  (img_w + (pxl_per_col_x-1) - offset_x)/pxl_per_col_x;
    // int min_box_x = -(img_h + (96-1)            + offset_y)/pxl_per_row_y;
    // int max_box_y =  (img_w + (pxl_per_row_x-1) - offset_x)/pxl_per_row_x
    //                 +(img_h + (96-1)            - offset_y)/pxl_per_row_y;
    // int min_box_y = -(offset_y/pxl_per_row_y)   -(offset_x /pxl_per_col_x);

    ImVec2 Top_Left;
    ImVec2 Origin;
    Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
    Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;
    ImVec2 Left, Top, Bottom, Right, new_origin;
    for (int y = min_box_y; y < max_box_y; y++)
    {
        for (int x = min_box_x; x < max_box_x; x++)
        {
            // new_origin.x = Origin.x + x * spacing_x * scale;
            // new_origin.y = Origin.y + y * spacing_y * scale;
            new_origin.x = Origin.x + x * offset->w * scale;
            new_origin.y = Origin.y + y * offset->h * scale;

            Top_Left.x = new_origin.x + (x* 48 + y*32)*scale;
            Top_Left.y = new_origin.y + (x*-12 + y*24)*scale;

            Left.x   = Top_Left.x + L_Corner.x*scale;
            Left.y   = Top_Left.y + L_Corner.y*scale;

            Top.x    = Top_Left.x + T_Corner.x*scale;
            Top.y    = Top_Left.y + T_Corner.y*scale;

            Right.x  = Top_Left.x + R_Corner.x*scale;
            Right.y  = Top_Left.y + R_Corner.y*scale;

            Bottom.x = Top_Left.x + B_Corner.x*scale;
            Bottom.y = Top_Left.y + B_Corner.y*scale;

            ImVec2 uv_l, uv_t, uv_r, uv_b, uv_ref;
            uv_ref.x = (float)(x* 48 + y*32 + offset->x);
            uv_ref.y = (float)(x*-12 + y*24 + offset->y);

            uv_l.x = (uv_ref.x + L_Corner.x) / img_data->width ;
            uv_l.y = (uv_ref.y + L_Corner.y) / img_data->height;
            uv_t.x = (uv_ref.x + T_Corner.x) / img_data->width ;
            uv_t.y = (uv_ref.y + T_Corner.y) / img_data->height;
            uv_r.x = (uv_ref.x + R_Corner.x) / img_data->width ;
            uv_r.y = (uv_ref.y + R_Corner.y) / img_data->height;
            uv_b.x = (uv_ref.x + B_Corner.x) / img_data->width ;
            uv_b.y = (uv_ref.y + B_Corner.y) / img_data->height;

            glBindTexture(GL_TEXTURE_2D, img_data->render_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            if (((uv_l.x >= 0) && (uv_l.x <= 1.0) && (uv_l.y >= 0) && (uv_l.y <= 1.0))
             || ((uv_t.x >= 0) && (uv_t.x <= 1.0) && (uv_t.y >= 0) && (uv_t.y <= 1.0))
             || ((uv_r.x >= 0) && (uv_r.x <= 1.0) && (uv_r.y >= 0) && (uv_r.y <= 1.0))
             || ((uv_b.x >= 0) && (uv_b.x <= 1.0) && (uv_b.y >= 0) && (uv_b.y <= 1.0)))
            {
                window->DrawList->AddImageQuad(
                    (ImTextureID)(uintptr_t)img_data->render_texture,
                    Left, Top, Right, Bottom,
                    uv_l, uv_t, uv_r, uv_b);

                // ImGui::ShowMetricsWindow();
                // printf("position: %d,%d\n", Left.x, Left.y);
            }

        }
    }

    // free(temp_buffer);
}


void export_button_table(tt_arr_handle* exported_tiles, user_info* usr_nfo, export_state* state)
{
    if (ImGui::BeginTable("auto_export", 2))
    {
//////////////////////////////////////////
        ImGui::TableNextColumn();
        //button 1
        if (exported_tiles == NULL) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Add to art/tiles/TILES.LST")) {
            ImGui::OpenPopup("Add FRMs to Mapper");
        }
        ImGui::SetItemTooltip(
            "TILES.LST located in:\n"
            "Fallout 2/data/art/tiles/\n\n"
            "Is checked for the names of these tiles\n"
            "and then appended to only if they\n"
            "don't already exist.\n\n"
            //TODO: delete this commented stuff, it works now
            // "(NOTE: Currently can't load\n"
            // "TILES.LST from master.dat\n"
            // "but should be able too in the future)"
        );
        if (exported_tiles == NULL) {
            ImGui::EndDisabled();
        }
        //checkbox 1
        ImGui::TableNextColumn();
        ImGui::Checkbox("Auto Append", &state->art);
        ImGui::SetItemTooltip(
            "Automatically appends\n"
            "exported FRMs to\n"
            "art/tiles/TILES.LST\n"
        );
        if (!state->art) {
            state->pro = false;
            state->pat = false;
        }

//////////////////////////////////////////
        ImGui::TableNextColumn();
        //button 2
        if (exported_tiles == NULL) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Export Protos")) {
            ImGui::OpenPopup("Proto Info");
        }
        if (exported_tiles == NULL) {
            ImGui::EndDisabled();
        }
        ImGui::TableNextColumn();
        //checkbox 2
        ImGui::Checkbox("Auto Export Protos", &state->pro);
        ImGui::SetItemTooltip(
            "Needs FRMs to be already listed\n"
            "in art/tiles/TILES.LST\n"
        );
        if (state->pro) {
            state->art = true;
        } else {
            state->pat = false;
        }
//////////////////////////////////////////
        ImGui::TableNextColumn();
        //button 3
        if (exported_tiles == NULL) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Export Pattern File")) {
            ImGui::OpenPopup("Pattern File");
        }
        if (exported_tiles == NULL) {
            ImGui::EndDisabled();
        }
        ImGui::TableNextColumn();
        //checkbox 3
        ImGui::Checkbox("Auto Export Pattern File", &state->pat);
        ImGui::SetItemTooltip(
            "Needs FRMs to be already listed\n"
            "in art/tiles/TILES.LST\n"
            "AND proto files to be exported\n"
            "and appended to proto/tiles/TILES.LST\n"
        );
        if (state->pat) {
            state->art      = true;
            state->pro      = true;
        }
//////////////////////////////////////////
        //individual popups
        if (ImGui::BeginPopupModal("Add FRMs to Mapper"))
        {
            append_FRM_tiles_POPUP(usr_nfo, exported_tiles, state, false);
            if (ImGui::Button("Close")) {
                set_false(state);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        // Popups: Always center this window when appearing
        //TODO: for some reason the centering doesn't work yet
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Proto Info", NULL, ImGuiWindowFlags_MenuBar))
        {
            export_PRO_tiles_POPUP(usr_nfo, exported_tiles, state, false);
            if (ImGui::Button("Close")) {
                set_false(state);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        // Always center this window when appearing? does this even work?
        //TODO: for some reason the centering doesn't work yet
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Pattern File", NULL, ImGuiWindowFlags_MenuBar))
        {
            export_PAT_file_POPUP(usr_nfo, exported_tiles, state, false);
            if (ImGui::Button("Close")) {
                set_false(state);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::EndTable();
    }
}


void rename_tiles(tt_arr_handle* handle, char* name)
{
    if (!handle) {
        return;
    }

    int tile_num = 0;
    tt_arr* tile = handle->tile;
    for (int i = 0; i < handle->size; i++) {
        if (tile[i].frm_id == -1) {
            continue;
        }
        snprintf(tile[i].name_ptr, 14, "%s%03d.FRM", name, tile_num++);
    }
}


void export_button_table_STATE(tt_arr_handle* exported_tiles, user_info* usr_nfo, STATE_export* state)
{
    if (ImGui::BeginTable("auto_export", 2))
    {
//////////////////////////////////////////
        ImGui::TableNextColumn();
        if (exported_tiles == NULL) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Add to art/tiles/TILES.LST")) {
            ImGui::OpenPopup("Add FRMs to Mapper");
        }
        ImGui::SetItemTooltip(
            "TILES.LST located in:\n"
            "Fallout 2/data/art/tiles/\n\n"
            "Is checked for the names of these tiles\n"
            "and then appended to only if they\n"
            "don't already exist.\n\n"
        );
        if (exported_tiles == NULL) {
            ImGui::EndDisabled();
        }
        //checkbox 1
        ImGui::TableNextColumn();
        ImGui::Checkbox("Auto Append", &state->art);
        ImGui::SetItemTooltip(
            "Automatically appends\n"
            "exported FRMs to\n"
            "art/tiles/TILES.LST\n"
        );
        if (!state->art) {
            state->pro = false;
            state->pat = false;
        }

//////////////////////////////////////////
        ImGui::TableNextColumn();
        //button 2
        if (exported_tiles == NULL) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Export NEW Protos (only)")) {
            ImGui::OpenPopup("Proto Info");
        }
        if (exported_tiles == NULL) {
            ImGui::EndDisabled();
        }
        ImGui::TableNextColumn();
        //checkbox 2
        ImGui::Checkbox("Auto Export Protos", &state->pro);
        ImGui::SetItemTooltip(
            "Needs FRMs to already be listed\n"
            "in art/tiles/TILES.LST\n"
        );
        if (state->pro) {
            state->art = true;
        } else {
            state->pat = false;
        }
//////////////////////////////////////////
        ImGui::TableNextColumn();
        //button 3
        if (exported_tiles == NULL) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Export Pattern File")) {
            ImGui::OpenPopup("Pattern File");
        }
        if (exported_tiles == NULL) {
            ImGui::EndDisabled();
        }
        ImGui::TableNextColumn();
        //checkbox 3
        ImGui::Checkbox("Auto Export Pattern File", &state->pat);
        ImGui::SetItemTooltip(
            "Needs FRMs to be already listed\n"
            "in art/tiles/TILES.LST\n"
            "AND proto files to be exported\n"
            "and appended to proto/tiles/TILES.LST\n"
        );
        if (state->pat) {
            state->art      = true;
            state->pro      = true;
        }
//////////////////////////////////////////
        //individual popups
        if (ImGui::BeginPopupModal("Add FRMs to Mapper"))
        {
            // append_FRM_tiles_POPUP(usr_nfo, exported_tiles, state, false);
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        // Popups: Always center this window when appearing (not sure this works)
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Proto Info", NULL, ImGuiWindowFlags_MenuBar))
        {
            // export_PRO_tiles_POPUP(usr_nfo, exported_tiles, state, false);
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        // Always center this window when appearing? does this even work?
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Pattern File", NULL, ImGuiWindowFlags_MenuBar))
        {
            // export_PAT_file_POPUP(usr_nfo, exported_tiles, state, false);
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::EndTable();
    }
}

bool load_FRM_LST_state(user_info* usr_nfo, STATE_export* state)
{
    char* LST_path = state->LST_path;
    snprintf(LST_path, MAX_PATH, "%s/data/art/tiles/TILES.LST", usr_nfo->default_game_path);
    char* actual_path = io_path_check(LST_path);
    if (actual_path) {
        strncpy(LST_path, actual_path, MAX_PATH);
    }

    char* FRM_tiles_lst = io_load_txt_file(LST_path);
    if (!FRM_tiles_lst) {
        return false;
    }

    if (usr_nfo->game_files.FRM_TILES_LST) {
        free(usr_nfo->game_files.FRM_TILES_LST);
    }
    usr_nfo->game_files.FRM_TILES_LST = FRM_tiles_lst;
    return true;

}

bool load_PRO_LST_state(user_info* usr_nfo, STATE_export* state)
{
    char* LST_path = state->LST_path;
    snprintf(LST_path, MAX_PATH, "%s/data/proto/tiles/TILES.LST", usr_nfo->default_game_path);
    char* actual_path = io_path_check(LST_path);
    if (actual_path) {
        strncpy(LST_path, actual_path, MAX_PATH);
    }

    char* old_PRO_LST = io_load_txt_file(LST_path);
    if (old_PRO_LST == nullptr) {
        printf("Unable to load /proto/tiles/TILES.LST...\n");
        return false;
    }

    if (usr_nfo->game_files.PRO_TILES_LST) {
        free(usr_nfo->game_files.PRO_TILES_LST);
    }
    usr_nfo->game_files.PRO_TILES_LST = old_PRO_LST;
    return true;

}

bool load_PRO_MSG_state(user_info* usr_nfo, STATE_export* state)
{
    char* LST_path = state->LST_path;
    snprintf(LST_path, MAX_PATH, "%s/data/text/%s/game/pro_tile.msg", usr_nfo->default_game_path, state->language[0]);
    char* actual_path = io_path_check(LST_path);
    if (actual_path) {
        strncpy(LST_path, actual_path, MAX_PATH);
    }

    char* old_PRO_MSG = io_load_txt_file(LST_path);
    if (old_PRO_MSG == nullptr) {
        //TODO: may want to handle other failures
        //      which would cause io_load_text_file()
        //      to return NULL/nullptr

        // ImGui::OpenPopup("Missing Files");


        printf("Unable to load /proto/tiles/TILES.LST...\n");
        return false;
    }

    if (usr_nfo->game_files.PRO_TILE_MSG) {
        free(usr_nfo->game_files.PRO_TILE_MSG);
    }
    usr_nfo->game_files.PRO_TILE_MSG = old_PRO_MSG;
    return true;
}



/*
button press to start checking files    CheckFiles
checking files success          --      ExportFiles
checking files failure          --      UserInput
user exit for failure           --      ExportMenu
user overwrite/extract          --      ExportFiles
*/
#include <queue>
#include <sml.hpp>
//events
class event_Render {};
class event_Export {};
class event_MatchesFound {};
class event_FilesNotFound {};
class event_UserExit {};
class event_UserExtract {};
class event_ResetState {};
class event_ShowError{};
//states
class state_ExportMenu {};
class state_CheckFiles {};
class state_UserInput {};
class state_ExtractFiles {};
class state_ExportFiles {};
class state_ErrorPopup {};

class ExportMachine {
    public:
    auto operator()() const {
        using namespace boost::sml;

        return make_transition_table(
            *state<class state_ExportMenu> + event<event_Render> /
                [](back::process<event_Export> process_event, const event_Render&, STATE_export* state) {
                    //this state is running when preview tiles window is open
                    //  but not when actually exporting

                    bool export_tile_popup = true;
                    if (ImGui::BeginPopupModal("Export Tiles", &export_tile_popup, ImGuiChildFlags_AutoResizeY)) {
                        //input name
                        ImGui::Text(
                            "Please type a default name for these tiles.\n"
                            "Exporting will append a tile number to this name.\n\n"
                            "Tile names can only be 8 characters total,\n"
                            "and 3 of those characters are currently\n"
                            "taken up by the numbering system.\n"
                            "(Which leaves 5 for you to work with).\n"
                            "ex: tile_000.FRM, tile_001.FRM, ... tile_999.FRM\n"
                        );
                        //game engine/mapper only takes 8 character tile-names
                        ImGui::InputText(
                            "Name\n(max 5 characters)",
                            state->save_name, 6);

                        // create the filename for the current list of tiles
                        // assigns final save path string to Full_Save_File_Path
                        // if (!auto_export) {
                        //     if (ImGui::Button("Save as Town Map Tiles")) {
                        //         save_folder_dialog(usr_info);
                        //     }
                        // }





                        ImGui::Text(
                            "In order to get new FRMs to appear in the Fallout 2\n"
                            "mapper (mapper2.exe), new entries must be made in\n\n"
                            "   Fallout 2/data/art/tiles/TILES.LST\n\n"
                            "For this to work, please provide the path to\n"
                            "fallout2.exe in your modded Fallout 2 folder,\n"
                            "and have this file extracted to its\n"
                            "appropriate location.\n"
                        );
                        //fallout2.exe check
                        //TODO: disable the "Auto Export All" button when fallout2.exe not found
                        static char FObuff[MAX_PATH] = "";
                        bool found = game_path_menu(state->usr_nfo, FObuff);

                        ImGui::Text(
                            "If only exporting FRMs, the mapper must be set in\n"
                            "'Librarian' mode and new protos must be made from\n"
                            "these new FRMs."
                        );

    static char* lst_path = NULL;

    //input name
    ImGui::Text(
        "In order to get new tiles to appear in the mapper\n"
        "(and thus in the game), each tile must have a proto(.pro)\n"
        "file made, and an entry for each tile appended to\n\n"
        "   Fallout 2/data/art/tiles/TILES.LST\n"
        "   Fallout 2/data/proto/tiles/TILES.LST\n\n"
        "In addition, entries can optionally be made in\n\n"
        "   Fallout 2/data/text/english/game/pro_tile.msg\n\n"
        "to give the tile a name and description in the\n"
        "Fallout 2 mapper (Mapper2.exe).\n\n"
    );

    ImGui::Text(
        "\nThese are Optional,\n"
        "and will be applied to all tiles in this set.\n"
    );

    get_material_id();
    input_name();
    input_desc();



                        if (state->art || state->pro || state->pat) {
                            if (found) {
                                ImGui::BeginDisabled();
                            }
                            if (ImGui::Button("Auto Export All")) {

                                //TODO: delete? are we using rename_tiles()?
                                // rename_tiles(state->handle, state->save_name);
                                state->handle = crop_TMAP_tiles(state->offset, state->src, state);
                                if (!state->handle) {
                                    //TODO: what do I do on fail?
                                } else {
                                    process_event(event_Export{});
                                }
                            }
                            if (found) {
                                ImGui::EndDisabled();
                            }
                            if (ImGui::Button("Close")) {
                                ImGui::CloseCurrentPopup();
                            }
                        }


                        ImGui::EndPopup();
                    }
                }
                ,

            *state<class state_ExportMenu> + event<event_Export>
                = state<class state_CheckFiles>,

            state<class state_CheckFiles>  + event<event_Render> /
                // render_check_files
                [](back::process<event_FilesNotFound, event_MatchesFound, event_Export> process_event, const event_Render&, STATE_export* state) {
                    bool FRM_LST = false;
                    bool PRO_LST = false;
                    bool PRO_MSG = false;
                    if (state->art) {
                        FRM_LST = load_FRM_LST_state(state->usr_nfo, state);
                    }
                    if (state->pro) {
                        PRO_LST = load_PRO_LST_state(state->usr_nfo, state);
                        PRO_MSG = load_PRO_MSG_state(state->usr_nfo, state);
                    }

                    bool match_found = false;
                    for (int i = 0; i < state->handle->size; i++)
                    {
                        if (state->handle->tile[i].frm_id == -1) {
                            continue;
                        }
                        snprintf(state->LST_path, MAX_PATH, "%s/data/art/tiles/%s",
                                state->usr_nfo->default_game_path,
                                state->handle->tile[i].name_ptr);

                        char* path_case = io_path_check(state->LST_path);

                        if (io_file_exists(path_case)) {
                            match_found = true;
                        }

                        if (match_found) {
                            process_event(event_MatchesFound{});
                        }
                    }

                    if (!FRM_LST || !PRO_LST || !PRO_MSG) {
                        process_event(event_FilesNotFound{});
                        ImGui::OpenPopup("Need Input!");
                    }
                    else
                    if (!match_found) {
                        process_event(event_Export{});
                    }
                }
                ,

            state<class state_CheckFiles>  + event<event_MatchesFound>
                = state<class state_ExportFiles>,
            state<class state_CheckFiles>  + event<event_FilesNotFound>
                = state<class state_UserInput>,
            state<class state_CheckFiles>   + event<event_Export>
                = state<class state_ExportFiles>,

            state<class state_UserInput>   + event<event_UserExit>
                = state<class state_ExportMenu>,

            state<class state_UserInput>   + event<event_Render> /
                // render_user_input
                [](back::process<event_Export, event_UserExtract, event_UserExit> process_event, const event_Render&, STATE_export* state) {
                    bool export_tile_popup = true;
                    bool open_popup = false;
                    if (ImGui::BeginPopupModal("Need Input!", &export_tile_popup, ImGuiChildFlags_AutoResizeY)) {
                        if (state->art || state->pro || state->pat) {
                            ImGui::Text("Unable to find:");
                            if (!state->usr_nfo->game_files.FRM_TILES_LST) {
                                ImGui::Text("art\\tiles\\TILES.LST");
                            }
                            if (state->pro) {
                                if (!state->usr_nfo->game_files.PRO_TILES_LST) {
                                    ImGui::Text("proto\\tiles\\TILES.LST");
                                }
                                if (!state->usr_nfo->game_files.PRO_TILE_MSG) {
                                    ImGui::Text("text\\english\\Game\\pro_tile.msg");
                                }
                            }

                            ImGui::Text("Extract from the relevant DAT file and append?\n");
                            //TODO: need to disable this button if fallout2.exe not found
                            if (ImGui::Button("Extract from DAT")) {
                                process_event(event_UserExtract{});
                            }

                            ImGui::Text(
                                "\n"
                                "Would you like to make new ones?\n"
                                "These new proto files will be blank\n"
                                "(except for the new tiles made here),\n"
                                "and will create all the subfolders\n"
                                "necessary for the game engine to load\n"
                                "these new files.\n\n"

                                "--IMPORTANT--\n"
                                "The Fallout game engine reads proto IDs/FRM names\n"
                                "in from *.LST files based on the line number.\n"
                                "The new *.LST files will override the old ones.\n"
                                "Only do this if you want to create\n"
                                "the whole tile system from scratch,\n"
                                "or to preview the results before manually merging.\n"
                            );
                            ImGui::BeginDisabled();
                            if (ImGui::Button("Create new blank LST")) {


                            }
                            ImGui::SameLine();
                            ImGui::Text("(Currently unimplemented)");
                            ImGui::EndDisabled();
                            if (ImGui::Button("Cancel")) {
                                ImGui::CloseCurrentPopup();
                                process_event(event_UserExit{});
                            }
                        }
                        ImGui::EndPopup();
                    }
                }
                ,


            state<class state_UserInput>   + event<event_UserExtract>
                = state<class state_ExtractFiles>,

            state<class state_ExtractFiles> + event<event_Render> /
                [](back::process<event_Render, event_ShowError> process_event, const event_Render&, STATE_export* state) {
                    if (tt_file_DAT_extract(state->usr_nfo, state)) {
                        ImGui::OpenPopup("Number 5 is Alive! Files Extracted!");
                        process_event(event_ShowError{});
                    } else {
                        ImGui::OpenPopup("Error: Unable to Extract Files");
                        process_event(event_ShowError{});
                    }
                }
                ,

            state<class state_ExportFiles> + event<event_Render> /
                [](back::process<event_Render, event_ShowError> process_event, const event_Render&, STATE_export* state) {
                    if (state->art) {
                        state->FRM_LST = _append_TMAP_tiles_LST(state->usr_nfo, state->handle);
                    }
                    if (state->pro) {
                        state->PRO_LST = _append_TMAP_PRO_tiles_LST(state->usr_nfo, state->handle);
                        //TODO: let the user choose the language
                        state->PRO_MSG = _append_PRO_tile_MSG(state->usr_nfo, state->handle, state->language[0]);
                    }

                    ImGui::OpenPopup("Append LST Files");
                    process_event(event_ShowError{});
                }
                ,

            state<class state_ExportFiles> + event<event_ShowError>
                = state<class state_ErrorPopup>,

            state<class state_ErrorPopup> + event<event_Render> /
                [](back::process<event_Render, event_ResetState, event_ShowError> process_event, const event_Render&, STATE_export* state) {

                    if (ImGui::BeginPopupModal("Append LST Files")) {
                        if (state->art) {
                            if (state->FRM_LST) {
                                ImGui::Text("Tile names appended to art/TILES.LST successfully.");
                            } else {
                                ImGui::Text("Something went wrong with art/TILES.LST, but I'm not sure what.");
                            }
                        }

                        if (state->pro) {
                            if (state->PRO_LST) {
                                ImGui::Text("Proto's successfully added to proto/TILES.LST");
                            } else {
                                ImGui::Text("Something went wrong with proto/TILES.LST, but I'm not sure what.");
                            }
                            if (state->PRO_MSG) {
                                ImGui::Text("Proto description successfully added to %s/game/pro_tile.msg", state->language[0]);
                            } else {
                                ImGui::Text("Something went wrong with %s/game/pro_tile.msg, but I'm not sure what.", state->language[0]);
                            }
                        }

                        if (ImGui::Button("Close")) {
                            ImGui::CloseCurrentPopup();
                            process_event(event_ResetState{});
                        }
                        ImGui::EndPopup();
                    }

                    if (ImGui::BeginPopupModal("Number 5 is Alive! Files Extracted!")) {
                        char* ptr = state->extracted;

                        int i = 0;
                        while (ptr[i] != '\0' && i < 4096)
                        {
                            ImGui::Text(&ptr[i]);
                            i += strlen(&ptr[i]) + 1;
                        }

                        ImGui::Text("Extracted from %s.dat");

                        if (ImGui::Button("Close")) {
                            process_event(event_ResetState{});
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }

                    if (ImGui::BeginPopupModal("Error: Unable to Extract Files")) {

                        ImGui::Text("Unable to extract:");
                        if (!state->usr_nfo->game_files.FRM_TILES_LST) {
                            ImGui::Text("art\\tiles\\TILES.LST");
                        }
                        if (state->pro) {
                            if (!state->usr_nfo->game_files.PRO_TILES_LST) {
                                ImGui::Text("proto\\tiles\\TILES.LST");
                            }
                            if (!state->usr_nfo->game_files.PRO_TILE_MSG) {
                                ImGui::Text("text\\english\\Game\\pro_tile.msg");
                            }
                        }

                        if (ImGui::Button("return to start?")) {
                            process_event(event_ResetState{});
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }

                }
                ,
            state<class state_ErrorPopup> + event<event_ResetState>
                = state<class state_ExportMenu>
        );
    }
};

enum TileExport {
    Off         , //= 0,
    Init        , //= 1,
    LoadFiles   , //= 2,
    MatchFound  , //= 3,
    Extract     , //= 4,
    Append      , //= 5,
    Save        , //= 6,
    MatchCheck  , //= 7,
    ExportTiles , //= 8,
    Feedback    , //= 9,
};

TileExport append_LST_feedback_popup(STATE_export* state, TileExport state_switch)
{
    if (ImGui::BeginPopupModal("Append LST Files")) {
        if (state->art) {
            if (state->FRM_LST) {
                ImGui::Text("Tile names appended to art/TILES.LST successfully.");
            } else {
                ImGui::Text("Something went wrong with art/TILES.LST, but I'm not sure what.");
            }
        }

        if (state->pro) {
            if (state->PRO_LST) {
                ImGui::Text("Proto's successfully added to proto/TILES.LST");
            } else {
                ImGui::Text("Something went wrong with proto/TILES.LST, but I'm not sure what.");
            }
            if (state->PRO_MSG) {
                ImGui::Text("Proto description successfully added to %s/game/pro_tile.msg", state->language[0]);
            } else {
                ImGui::Text("Something went wrong with %s/game/pro_tile.msg, but I'm not sure what.", state->language[0]);
            }
        }

        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
            state_switch = Off;
            // process_event(event_ResetState{});
        }
        ImGui::EndPopup();
    }
    return state_switch;
}

TileExport extract_LST_feedback_popup(STATE_export* state, TileExport state_switch)
{
    if (ImGui::BeginPopupModal("Number 5 is Alive! Files Extracted!")) {
        char* ptr = state->extracted;

        int i = 0;
        while (ptr[i] != '\0' && i < 4096)
        {
            ImGui::Text(&ptr[i]);
            i += strlen(&ptr[i]) + 1;
        }

        ImGui::Text("Extracted from %s.dat");

        if (ImGui::Button("Close")) {
            // process_event(event_ResetState{});
            state_switch = Off;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return state_switch;
}

TileExport extract_LST_fail_popup(STATE_export* state, TileExport state_switch)
{
    if (ImGui::BeginPopupModal("Error: Unable to Extract Files")) {

        ImGui::Text("Unable to extract:");
        if (!state->usr_nfo->game_files.FRM_TILES_LST) {
            ImGui::Text("art\\tiles\\TILES.LST");
        }
        if (state->pro) {
            if (!state->usr_nfo->game_files.PRO_TILES_LST) {
                ImGui::Text("proto\\tiles\\TILES.LST");
            }
            if (!state->usr_nfo->game_files.PRO_TILE_MSG) {
                ImGui::Text("text\\english\\Game\\pro_tile.msg");
            }
        }

        if (ImGui::Button("return to start?")) {
            // process_event(event_ResetState{});
            state_switch = Off;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return state_switch;
}

TileExport export_TILES_success_popup(STATE_export* state, TileExport state_switch)
{
    if (ImGui::BeginPopupModal("Export Successful", NULL, ImGuiChildFlags_AlwaysAutoResize)) {

        ImGui::BeginChild("Export Successful", ImVec2(ImGui::GetContentRegionAvail().x, 260), ImGuiChildFlags_None);
        for (int i = 0; i < state->handle->size; i++)
        {
            tt_arr* tile = &state->handle->tile[i];
            if (tile->frm_id != -1) {
                ImGui::Text("%s\n", tile->name_ptr);
            }
        }
        ImGui::EndChild();

        ImGui::Text("Exported Successfully to\n%s\n", state->save_path);

        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
            state_switch = Off;
        }

        ImGui::EndPopup();
    }
    return state_switch;
}

// Feedback
TileExport export_TILE_feedback(STATE_export* state)
{
    TileExport state_switch = Feedback;

    state_switch = append_LST_feedback_popup( state, state_switch);
    state_switch = extract_LST_feedback_popup(state, state_switch);
    state_switch = extract_LST_fail_popup(    state, state_switch);
    state_switch = export_TILES_success_popup(state, state_switch);

    return state_switch;
}

// Append
TileExport export_TILE_append(STATE_export* state)
{
    TileExport state_switch = Append;
    if (state->art) {
        state->FRM_LST = _append_TMAP_tiles_LST(state->usr_nfo, state->handle);
    }
    if (state->pro) {
        state->PRO_LST = _append_TMAP_PRO_tiles_LST(state->usr_nfo, state->handle);
        //TODO: let the user choose the language
        state->PRO_MSG = _append_PRO_tile_MSG(state->usr_nfo, state->handle, state->language[0]);
    }

    ImGui::OpenPopup("Append LST Files");
    // process_event(event_ShowError{});
    state_switch = Feedback;
    return state_switch;
}

// Extract
TileExport export_TILE_success(STATE_export* state)
{
    if (tt_file_DAT_extract(state->usr_nfo, state)) {
        ImGui::OpenPopup("Number 5 is Alive! Files Extracted!");
        // process_event(event_ShowError{});
    } else {
        ImGui::OpenPopup("Error: Unable to Extract Files");
        // process_event(event_ShowError{});
    }
    return Feedback;
}

// MatchFound
TileExport export_TILE_match_found_popup(STATE_export* state)
{
    TileExport state_switch = MatchFound;
    bool need_input_popup = true;
    bool match_found_popup = true;
    bool open_popup = false;
    if (ImGui::BeginPopupModal("Need Input!", &need_input_popup, ImGuiChildFlags_AutoResizeY)) {
        if (state->art) {
            ImGui::Text("Unable to find:");
            if (!state->usr_nfo->game_files.FRM_TILES_LST) {
                ImGui::Text("art\\tiles\\TILES.LST");
            }
            if (state->pro) {
                if (!state->usr_nfo->game_files.PRO_TILES_LST) {
                    ImGui::Text("proto\\tiles\\TILES.LST");
                }
                if (!state->usr_nfo->game_files.PRO_TILE_MSG) {
                    ImGui::Text("text\\english\\Game\\pro_tile.msg");
                }
            }

            ImGui::Text("Extract from the relevant DAT file and append?\n");
            //TODO: need to disable this button if fallout2.exe not found
            if (ImGui::Button("Extract from DAT")) {
                state_switch = Extract;
                ImGui::CloseCurrentPopup();
            }

            ImGui::Text(
                "\n"
                "Would you like to make new ones?\n"
                "These new proto files will be blank\n"
                "(except for the new tiles made here),\n"
                "and will create all the subfolders\n"
                "necessary for the game engine to load\n"
                "these new files.\n\n"

                "--IMPORTANT--\n"
                "The Fallout game engine reads proto IDs/FRM names\n"
                "in from *.LST files based on the line number.\n"
                "The new *.LST files will override the old ones.\n"
                "Only do this if you want to create\n"
                "the whole tile system from scratch,\n"
                "or to preview the results before manually merging.\n"
            );
            ImGui::BeginDisabled();
            if (ImGui::Button("Create new blank LST")) {
                //TODO: get this working
            }
            ImGui::SameLine();
            ImGui::Text("(Currently unimplemented)");
            ImGui::EndDisabled();
            if (ImGui::Button("Cancel")) {
                state_switch = Off;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Match Found", &match_found_popup, ImGuiChildFlags_AutoResizeY)) {
        ImGui::Text("Filename matches found:\n%s\n", state->matches);

        if (ImGui::Button("Overwrite?")) {
            state_switch = ExportTiles;
            ImGui::CloseCurrentPopup();
            free(state->matches);
        }
        if (ImGui::Button("Select a different folder?")) {
            state_switch = Save;
            ImGui::CloseCurrentPopup();
            free(state->matches);
        }
        if (ImGui::Button("Cancel")) {
            state_switch = Init;
            ImGui::CloseCurrentPopup();
            free(state->matches);
        }

        ImGui::EndPopup();
    }

    return state_switch;
}

// LoadFiles
TileExport export_TILE_load_LST_files(STATE_export* state)
{
    TileExport switch_state = LoadFiles;
    bool FRM_LST = false;
    bool PRO_LST = false;
    bool PRO_MSG = false;
    if (state->art) {
        FRM_LST = load_FRM_LST_state(state->usr_nfo, state);
    }
    if (state->pro) {
        PRO_LST = load_PRO_LST_state(state->usr_nfo, state);
        PRO_MSG = load_PRO_MSG_state(state->usr_nfo, state);
    }

    // bool match_found = false;
    // for (int i = 0; i < state->handle->size; i++)
    // {
    //     if (state->handle->tile[i].frm_id == -1) {
    //         continue;
    //     }
    //     snprintf(state->LST_path, MAX_PATH, "%s/data/art/tiles/%s",
    //             state->usr_nfo->default_game_path,
    //             state->handle->tile[i].name_ptr);
    //     char* path_case = io_path_check(state->LST_path);
    //     if (io_file_exists(path_case)) {
    //         match_found = true;
    //     }
    // }
    // if (match_found) {
    //     // process_event(event_MatchesFound{});
    //     switch_state = MatchFound;
    //     ImGui::OpenPopup("Match Found");
    // }

    if (state->art) {
        if (FRM_LST) {
            // process_event(event_FilesNotFound{});
            switch_state = Append;
        } else {
            switch_state = Extract;
        }
    }
    if (state->pro) {
        if (PRO_LST && PRO_MSG) {
            switch_state = Append;
        } else {
            switch_state = Extract;
        }

    }
    if (switch_state == Extract) {
        // process_event(event_Export{});
        ImGui::OpenPopup("Need Input!");
    }
    return switch_state;
}

// Init
TileExport export_TILE_init_popup(STATE_export* state)
{
    TileExport state_switch = Init;

    bool export_tile_popup = true;
    if (ImGui::BeginPopupModal("Export Tiles", &export_tile_popup, ImGuiChildFlags_AutoResizeY)) {
        export_button_table_STATE(state->handle, state->usr_nfo, state);
        //input name
        ImGui::Text(
            "Please type a default name for these tiles.\n"
            "Exporting will append a tile number to this name.\n\n"
            "Tile names can only be 8 characters total,\n"
            "and 3 of those characters are currently\n"
            "taken up by the numbering system.\n"
            "(Which leaves 5 for you to work with).\n"
            "ex: tile_000.FRM, tile_001.FRM, ... tile_999.FRM\n"
        );
        //game engine/mapper only takes 8 character tile-names
        ImGui::InputText(
            "Name\n(max 5 characters)",
            state->save_name, 6);

        bool found = false;

        if (state->art) {   // || state->pro || state->pat) {
            ImGui::Text(
                "In order to get new FRMs to appear in the Fallout 2\n"
                "mapper (mapper2.exe), new entries must be made in\n\n"
                "   Fallout 2/data/art/tiles/TILES.LST\n\n"
                "For this to work, please provide the path to\n"
                "fallout2.exe in your modded Fallout 2 folder,\n"
                "and have this file extracted to its\n"
                "appropriate location.\n"
            );
            //fallout2.exe check
            //TODO: disable the "Auto Export All" button when fallout2.exe not found
            static char FObuff[MAX_PATH] = "";
            found = game_path_menu(state->usr_nfo, FObuff);
        }

        ImGui::Text(
            "If only exporting FRMs, the mapper must be set in\n"
            "'Librarian' mode and new protos must be made from\n"
            "inside the mapper, using these new FRMs."
        );

        if (state->pro) {
            //input name
            ImGui::Text(
            "In order to get new tiles to appear in the mapper\n"
            "(and thus in the game), each tile must have a proto(.pro)\n"
            "file made, and an entry for each tile appended to\n\n"
            "   Fallout 2/data/art/tiles/TILES.LST\n"
            "   Fallout 2/data/proto/tiles/TILES.LST\n\n"
            "In addition, entries can optionally be made in\n\n"
            "   Fallout 2/data/text/english/game/pro_tile.msg\n\n"
            "to give the tile a name and description in the\n"
            "Fallout 2 mapper (Mapper2.exe).\n\n"
            );

            ImGui::Text(
            "\nThese are Optional,\n"
            "and will be applied to all tiles in this set.\n"
            );

            get_material_id();
            input_name();
            input_desc();
        }

        if (state->art) {   // || state->pro || state->pat) {
            if (found) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("Auto Export All")) {

                //TODO: delete? are we using rename_tiles()?
                // rename_tiles(state->handle, state->save_name);
                state->handle = crop_TMAP_tiles(state->offset, state->src, state);
                if (state->handle) {
                    // state_switch = LoadFiles;
                    static char buffer[MAX_PATH];
                    snprintf(buffer, MAX_PATH, "%s/data/ART/tiles", state->usr_nfo->default_game_path);
                    // state->save_path = state->usr_nfo->default_game_path;
                    state->save_path = buffer;
                    state_switch = MatchCheck;
                } else {
                    //TODO: what do I do on fail? warning popup?
                    // state_switch=Feedback    //?
                }
            }
            if (found) {
                ImGui::EndDisabled();
            }
            if (ImGui::Button("Close")) {
                state_switch = Off;
                ImGui::CloseCurrentPopup();
            }
        } else {
            if (ImGui::Button("Auto Export Tile FRMs")) {
                state->handle = crop_TMAP_tiles(state->offset, state->src, state);
                if (!state->handle) {
                    //TODO: what do I do on fail?
                } else {
                    state_switch = Save;
                }
            }
        }
        ImGui::EndPopup();
    }
    return state_switch;
}

TileExport export_TILE_save(STATE_export* state)
{
    TileExport state_switch = Save;

    char* save_folder = ImDialog_save_folder(state->usr_nfo);

    if (save_folder) {
        if (save_folder[0] == '\0') {
            state_switch = Init;
            ImGui::OpenPopup("Export Tiles");
        } else {
            state->save_path = save_folder;
            state_switch = MatchCheck;
        }
    }

    return state_switch;
}

TileExport export_TILE_export(STATE_export* state)
{
    TileExport switch_state = ExportTiles;
    char* path = io_path_check(state->save_path);
    if (path != state->save_path) {
        strncpy(state->save_path, path, MAX_PATH);
    }
    bool success = io_make_dir(state->save_path);

    tt_arr_handle* handle = NULL;
    if (state->save_path[0] != '\0' && success) {
        handle = export_TMAP_tiles(state->offset, state->handle, state->save_path);
        if (!handle) {
            success = false;
        }
    }

    if (success) {
        // ImGui::OpenPopup("Export Successful");
        // switch_state = Feedback;
        switch_state = LoadFiles;
    }

    return switch_state;
}

char* check_names(char* path, tt_arr_handle* handle)
{
    int match_len = 0;
    char* matches = NULL;
    char buffer[MAX_PATH];
    int size = handle->size;
    for (int i = 0; i < size; i++)
    {
        tt_arr* tile = &handle->tile[i];
        if (tile->frm_id == -1) {
            continue;
        }

        snprintf(buffer, MAX_PATH, "%s/%s", path, tile->name_ptr);

        if (io_file_exists(buffer)) {
            if (!matches) {
                match_len = 64;
                matches = (char*)calloc(1,64);
            } else
            if (strlen(matches) + 8 > match_len) {
                matches = (char*)realloc(matches, match_len + 64);
                match_len += 64;
            }
            strcat(matches, tile->name_ptr);
            strcat(matches, "\n");
        }
    }
    return matches;
}

TileExport export_TILE_check_names(STATE_export* state)
{
    TileExport switch_state = MatchCheck;
    char* path = state->save_path;

    state->matches = check_names(state->save_path, state->handle);
    if (state->matches) {
        switch_state = MatchFound;
        ImGui::OpenPopup("Match Found");
    } else {
        switch_state = ExportTiles;
    }

    return switch_state;
}

void export_TILE_state_machine(STATE_export* state)
{
    static TileExport state_switch = Off;

    if (ImGui::Button("Export Selected")) {
        ImGui::OpenPopup("Export Tiles");
        state_switch = Init;
    }

    switch (state_switch)
    {
    case Off:
        //do nothing
        break;
    case Init:
        state_switch = export_TILE_init_popup(state);
        break;
    case LoadFiles:
        state_switch = export_TILE_load_LST_files(state);
        break;
    case MatchCheck:
        state_switch = export_TILE_check_names(state);
        break;
    case MatchFound:
        state_switch = export_TILE_match_found_popup(state);
        break;
    case Extract:
        state_switch = export_TILE_success(state);
        break;
    case Append:
        state_switch = export_TILE_append(state);
        break;
    case Save:
        state_switch = export_TILE_save(state);
        break;
    case ExportTiles:
        state_switch = export_TILE_export(state);
        break;
    case Feedback:
        state_switch = export_TILE_feedback(state);
        break;

    default:
        printf("got a missing state: L%d\n", __LINE__);
        break;
    }
}

tt_arr_handle* TMAP_tile_state_machine(user_info* usr_nfo, Surface* srfc, Rect* offset, tt_arr_handle* handle)
{
    static STATE_export state;
    state.usr_nfo = usr_nfo;
    state.offset  = offset;
    state.src     = srfc;
    // static boost::sml::sm<ExportMachine, boost::sml::process_queue<std::queue>> StateMachine{&state};
    export_TILE_state_machine(&state);

    if (handle) {
        state.handle = handle;
    }






    //TODO: move these into their own function (plus add stuff)
    ImGui::SliderInt("Image Offset X", &offset->x, -400, 400, NULL);
    ImGui::SliderInt("Image Offset Y", &offset->y, -400, 400, NULL);
    ImGui::SliderInt("Tile Spacing X", &offset->w, 0, 80, NULL);
    ImGui::SliderInt("Tile Spacing Y", &offset->h, 0, 80, NULL);

    // StateMachine.process_event(event_Render{});

    return state.handle;

}


tt_arr_handle* TMAP_tile_buttons(user_info* usr_nfo, Surface* srfc, Rect* offset, tt_arr_handle* handle)
{
    static export_state state;

    static tt_arr_handle* exported_tiles = NULL;
    if (handle) {
        exported_tiles = handle;
    }
    //Save tiles button
    const char* export_txt = "Export Tile FRMs only";
    if (state.art) {
        export_txt = "Export Selected";
    }
    if (ImGui::Button(export_txt)) {
        ImGui::OpenPopup("Export Tiles");
    }

    bool export_tile_popup = true;
    if (ImGui::BeginPopupModal("Export Tiles", &export_tile_popup, ImGuiChildFlags_AutoResizeY)) {
        if (state.art || state.pro || state.pat) {
        //TODO: need to disable this button if fallout2.exe not found
            if (ImGui::Button("Auto Export All")) {
                if (state.art) {
                    state.auto_export    = true;
                    state.load_files     = true;
                    state.append_FRM_LST = true;
                }
                if (state.pro) {
                    state.export_proto   = true;
                    state.append_PRO_LST = true;
                    state.append_PRO_MSG = true;
                }
                if (state.pat) {
                    state.export_pattern = true;
                }
                rename_tiles(exported_tiles, state.save_name);
            }
        }

        if (ImGui::Button("Close")) {
            set_false(&state);
            ImGui::CloseCurrentPopup();
        }

        tt_arr_handle* temp = export_TMAP_tiles_POPUP(usr_nfo, srfc, offset, &state);
        if (temp) {
            //assign handle only if tiles have been fully exported
            //pressing cancel won't clear old handle
            exported_tiles = temp;
        }

        if (state.art) {
            append_FRM_tiles_POPUP(usr_nfo, exported_tiles, &state, true);
        }
        if (state.pro) {
            export_PRO_tiles_POPUP(usr_nfo, exported_tiles, &state, true);
        }
        if (state.pat) {
            export_PAT_file_POPUP(usr_nfo, exported_tiles, &state, true);
        }

        ImGui::EndPopup();

        if (state.auto_export == true) {
            ImGui::OpenPopup("Export Complete");
        }
        set_false(&state);
    }

    bool export_success = true;
    if (ImGui::BeginPopupModal("Export Complete", &export_success)) {
        ImGui::Text(
            "Tiles exported successfully."
        );
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    export_button_table(exported_tiles, usr_nfo, &state);


    ImGui::SliderInt("Image Offset X", &offset->x, -400, 400, NULL);
    ImGui::SliderInt("Image Offset Y", &offset->y, -400, 400, NULL);

    ImGui::SliderInt("Tile Spacing X", &offset->w, 0, 80, NULL);
    ImGui::SliderInt("Tile Spacing Y", &offset->h, 0, 80, NULL);

    if (exported_tiles != handle) {
        return exported_tiles;
    }
    return NULL;
}

void prev_TMAP_tiles_SURFACE(user_info* usr_info, variables *My_Variables, image_data *img_data)
{
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);
    shader_info *shaders = &My_Variables->shaders;


    int dir = img_data->display_orient_num;
    if (!img_data->ANM_dir) {
        ImGui::Text("No ANM_dir");
        return;
    }
    if (img_data->ANM_dir[dir].frame_data == NULL) {
        ImGui::Text("No frame_data");
        return;
    }

    animate_SURFACE_to_sub_texture(
        img_data, img_data->ANM_dir[dir].frame_data[0],
        My_Variables->CurrentTime_ms
    );

    //TODO: rename?
    //      this takes 3 textures and draws them into 1 framebuffer
    draw_PAL_to_framebuffer(
        shaders->FO_pal,
        shaders->render_PAL_shader,
        &shaders->giant_triangle,
        img_data);

    static bool image_toggle = false;
    checkbox_handler("toggle image", &image_toggle);
    if (image_toggle)
    {
        ImVec2 uv_min = {0, 0};
        ImVec2 uv_max = {1.0, 1.0};
        int width   = img_data->width;
        int height  = img_data->height;
        float scale = img_data->scale;
        ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));

        ImGuiWindow *window = ImGui::GetCurrentWindow();
    //TODO: change top_corner() for img_pos passed in from outside
        window->DrawList->AddImage(
            (ImTextureID)(uintptr_t)img_data->render_texture,
            top_corner(img_data->offset), bottom_corner(size, top_corner(img_data->offset)),
            uv_min, uv_max,
            ImGui::GetColorU32(My_Variables->tint_col));
    }

    show_popup_warnings();

    static Rect offset = {};
    static tt_arr_handle* handle = nullptr;
    Surface* srfc = img_data->ANM_dir[dir].frame_data[0];
    // handle = TMAP_tile_buttons(usr_info, srfc, &offset, handle);

    handle = TMAP_tile_state_machine(usr_info, srfc, &offset, handle);

    draw_TMAP_tiles(usr_info, img_data, shaders,
                    My_Variables->tile_texture_rend,
                    &offset);
}

void draw_red_squares(image_data *img_data, bool show_squares)
{
    // Draw red boxes to indicate where the tiles will be cut from
    float scale = img_data->scale;
    if (show_squares)
    {
        ImDrawList *Draw_List = ImGui::GetWindowDrawList();
        ImVec2 Origin;
        Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
        Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

        ImVec2 Top_Left;
        ImVec2 Bottom_Right = {0, 0};
        int max_box_x = img_data->width  / 350;
        int max_box_y = img_data->height / 300;

        for (int j = 0; j < max_box_y; j++)
        {
            for (int i = 0; i < max_box_x; i++)
            {
                Top_Left.x = Origin.x + (i * 350) * scale;
                Top_Left.y = Origin.y + (j * 300) * scale;
                Bottom_Right = {(float)(Top_Left.x + 350 * scale), (float)(Top_Left.y + 300 * scale)};
                Draw_List->AddRect(Top_Left, Bottom_Right, 0xff0000ff, 0, 0, 5.0f);
            }
        }
    }
}

// struct to hold the 4 points for the quadrilateral (tile shape or image shape)
struct outline
{
    ImVec2 Top,
        Rgt,
        Btm,
        Lft;
};

// add offset to each point of square
void add_offset(ImVec2 offset, outline *square)
{
    square->Top.x += offset.x;
    square->Top.y += offset.y;
    square->Rgt.x += offset.x;
    square->Rgt.y += offset.y;
    square->Btm.x += offset.x;
    square->Btm.y += offset.y;
    square->Lft.x += offset.x;
    square->Lft.y += offset.y;
}

//TODO: remove these next few functions
//      they were used to draw red tiles
//      on the original image
#define TMAP_W (80 + 48)
#define TMAP_H (36 + 24)
// draw tiles for the preview screen when checking the tiles box
void draw_red_tiles(image_data *img_data, bool show_squares)
{
    // Draw red boxes to indicate where the tiles will be cut from
    float scale = img_data->scale;
    if (!show_squares)
    {
        return;
    }

    ImDrawList *Draw_List = ImGui::GetWindowDrawList();
    ImVec2 Origin;
    Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
    Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

    ImVec2 Top_Left;
    outline tile_offsets;

    tile_offsets.Top = {48 * scale, -12 * scale};
    tile_offsets.Rgt = {80 * scale,  12 * scale};
    tile_offsets.Btm = {32 * scale,  24 * scale};
    tile_offsets.Lft = {00 * scale,  00 * scale};

    int max_box_x = img_data->width / TMAP_W;
    int max_box_y = img_data->height / TMAP_H;

    static int offset1;
    static int offset2;
    static int offset3;
    static int offset4;
    ImGui::SliderInt("offset1", &offset1, -80, 80, NULL);
    ImGui::SliderInt("offset2", &offset2, -80, 80, NULL);
    ImGui::SliderInt("offset3", &offset3, -80, 80, NULL);
    ImGui::SliderInt("offset4", &offset4, -80, 80, NULL);

    Origin.x += offset3 * scale;

    ImVec2 offset =     { 48 * scale + scale * offset1,
                         -12 * scale + scale * offset2};
    ImVec2 row_offset = {-16 * scale + scale * offset4,
                          36 * scale + scale * offset4};

    Top_Left.x = Origin.x - 32 * scale;
    Top_Left.y = Origin.y;

    outline row_start;                 // used when incrementing rows
    outline new_square;                // stores tile position for each tile
    new_square = tile_offsets;         // copy default dimensions
    add_offset(Top_Left, &new_square); // move first tile position to image corner
    row_start = new_square;            // copy this position for re-use

    int img_right = Origin.x + img_data->width * scale;
    int img_bottom = Origin.y + img_data->height * scale;
    bool drew_row = true;
    int count = 0;

    while (true)
    {
        new_square = row_start;

        // when you switch to the next row, after doing the current addition,
        // you could "advance" the start of the row by regular tile offset
        //(h_offset and v_offset) until it's inside the image,
        // or past the end of it
        // to test it's working correctly, you could comment out the if
        // condition wrapping the draw quad
        // And you'd break if you couldn't "find" the start of the row
        //(i.e. the left point is past the right edge of the image)

        if (!drew_row)
        {
            printf("count: %d\n", count);
            break;
        }
        drew_row = false;

        while (true)
        {
            if ((new_square.Top.y < img_bottom) && // crop bottom
                (new_square.Rgt.x > Origin.x))
            { // crop left
                Draw_List->AddQuad(new_square.Lft,
                                   new_square.Btm,
                                   new_square.Rgt,
                                   new_square.Top, 0xff0000ff, 1.0f);
                drew_row = true;
                count++;
            }

            add_offset(offset, &new_square);

            if (new_square.Btm.y < (Origin.y))
            { // crop top
                break;
            }
            if (new_square.Lft.x > (img_right))
            { // crop right
                break;
            }
        }

        add_offset(row_offset, &row_start);
    }
}

void draw_tiles_OpenGL(image_data *img_data, shader_info *shader, GLuint *texture, bool draw_tiles)
{
    float scale = img_data->scale;

    printf("draw_tiles_OpenGL is being called...\n");

    if (draw_tiles)
    {
        ImVec2 Origin;
        Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
        Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

        glViewport(0, 0, img_data->width, img_data->height);
        glBindFramebuffer(GL_FRAMEBUFFER, img_data->framebuffer);
        glBindVertexArray(shader->giant_triangle.VAO);
        glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 96, GL_RED, GL_UNSIGNED_BYTE, texture);

        glDrawArrays(GL_TRIANGLES, 0, shader->giant_triangle.vertexCount);

        // bind framebuffer back to default
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
