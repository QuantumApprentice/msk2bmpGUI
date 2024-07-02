//#define _CRTDBG_MAP_ALLOC
////#define SDL_MAIN_HANDLED
//
//#define SET_CRT_DEBUG_FIELD(a) \
//    _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
//#define CLEAR_CRT_DEBUG_FIELD(a) \
//    _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#define SDL_MAIN_HANDLED

// ImGui header files
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#include <stdio.h>
#include <SDL.h>
#include <SDL_blendmode.h>
#include <glad/glad.h>

//TODO: fix this so it compiles for windows
#ifdef QFO2_WINDOWS
    #include <Windows.h>
#endif


#include <fstream>
#include <sstream>
#include <iostream>
#include <optional>

// My header files
#include "platform_io.h"
#include "Load_Files.h"
#include "FRM_Convert.h"
#include "Save_Files.h"
#include "Image2Texture.h"
#include "Load_Settings.h"
#include "Edit_Image.h"
#include "Preview_Image.h"
#include "tinyfiledialogs.h"

#include "display_FRM_OpenGL.h"
#include "Palette_Cycle.h"
#include "Image_Render.h"
#include "Preview_Tiles.h"
#include "MSK_Convert.h"
#include "Edit_Animation.h"

#include "timer_functions.h"

//remove
#include "B_Endian.h"

// Our state
user_info usr_info;

// Function declarations
void Show_Preview_Window(variables *My_Variables, int counter, SDL_Event* event); //, SDL_Renderer* renderer);
void Preview_Tiles_Window(variables *My_Variables, int counter);
// void Preview_Town_Tiles_Window(variables *My_Variables, int counter);
void Show_Image_Render(variables *My_Variables, struct user_info* usr_info, int counter);
void Edit_Image_Window(variables *My_Variables, struct user_info* usr_info, int counter);

void Show_Palette_Window(struct variables *My_Variables);

static void ShowMainMenuBar(int* counter, struct variables* My_Variables);
void Open_Files(struct user_info* usr_info, int* counter, SDL_PixelFormat* pxlFMT, struct variables* My_Variables);

void contextual_buttons(variables* My_Variables, int window_number_focus);
void Show_MSK_Palette_Window(variables* My_Variables);
void popup_save_menu(bool* open_window, int* save_type, bool* single_dir);



// Main code
int main(int argc, char** argv)
{
    int my_argc;

#ifdef QFO2_WINDOWS
    LPWSTR* my_argv = CommandLineToArgvW(GetCommandLineW(), &my_argc);
#elif defined(QFO2_LINUX)
    char** my_argv = argv;
#endif


    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
    SDL_GL_LoadLibrary(NULL);

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 330 core";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Q's Beta Fallout Image Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        printf("Glad Loader failed?..."); // %s", SDL_GetVideoDriver);
        exit(-1);
    }
    else
    {
        printf("Vendor: %s\n",       glGetString(GL_VENDOR));
        printf("Renderer: %s\n",     glGetString(GL_RENDERER));
        printf("Version: %s\n",      glGetString(GL_VERSION));
        printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    }

    SDL_GL_SetSwapInterval(0); // Enable vsync

    //State variables
    struct variables My_Variables = {};
    My_Variables.exe_directory = Program_Directory();
    usr_info.exe_directory     = My_Variables.exe_directory;

    char vbuffer[MAX_PATH];
    char fbuffer[MAX_PATH];
    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources//shaders//passthru_shader.vert");
    snprintf(fbuffer, sizeof(fbuffer), "%s%s", My_Variables.exe_directory, "resources//shaders//render_PAL.frag");
    My_Variables.shaders.render_PAL_shader = new Shader(vbuffer, fbuffer);

    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources//shaders//passthru_shader.vert");
    snprintf(fbuffer, sizeof(fbuffer), "%s%s", My_Variables.exe_directory, "resources//shaders//render_FRM.frag");
    My_Variables.shaders.render_FRM_shader = new Shader(vbuffer, fbuffer);

    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources//shaders//passthru_shader.vert");
    snprintf(fbuffer, sizeof(fbuffer), "%s%s", My_Variables.exe_directory, "resources//shaders//passthru_shader.frag");
    My_Variables.shaders.render_OTHER_shader = new Shader(vbuffer, fbuffer);

    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources\\grid-texture.png");
    load_tile_texture(&My_Variables.tile_texture_prev, vbuffer);
    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources\\blue_tile_mask.png");
    load_tile_texture(&My_Variables.tile_texture_rend, vbuffer);

    //TODO: add user input for a user-specified palette
    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources/palette/color.pal");
    My_Variables.pxlFMT_FO_Pal = load_palette_to_SDL_PixelFormat(vbuffer);


    Load_Config(&usr_info, My_Variables.exe_directory);

    //My_Variables.pxlFMT_FO_Pal = loadPalette("file name for palette here");
    bool success = load_palette_to_float_array(My_Variables.shaders.palette, My_Variables.exe_directory);
    if (!success) { printf("failed to load palette to float array for OpenGL\n"); }

    My_Variables.shaders.giant_triangle = load_giant_triangle();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "imgui.ini");
    //io.IniFilename = vbuffer;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);


    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !

    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources//fonts//OpenSans-Bold.ttf");
    io.Fonts->AddFontDefault();
    My_Variables.Font = io.Fonts->AddFontFromFileTTF(vbuffer, My_Variables.global_font_size);

    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    //this counter is used to identify which image slot is being used for now
    //TODO: need to swap this for a linked list
    static int counter = 0;



#ifdef QFO2_WINDOWS
    if (my_argv == NULL) {
        MessageBox(NULL,
            "Something went wrong?",
            "argv is NULL?",
            MB_ABORTRETRYIGNORE);
    }
    else {
        //for (int i = 0; i < my_argc; i++)
        //{
        //    MessageBoxW(NULL,
        //        my_argv[i],
        //        L"AAARrrrrrgggghhhhv",
        //        MB_ABORTRETRYIGNORE);
        //}
        if (my_argc > 1) {
            handle_file_drop(tinyfd_utf16to8(my_argv[1]),
                &My_Variables.F_Prop[counter],
                &counter,
                &My_Variables.shaders);
        }
    }
#elif defined(QFO2_LINUX)
    if (my_argc > 1) {
        handle_file_drop(my_argv[1],
            &My_Variables.F_Prop[counter],
            &counter,
            &My_Variables.shaders);
    }
#endif


    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // used to reset the default layout back to original
    bool firstframe = true;
    std::vector<std::string> dropped_file_path;

    // Main loop
    bool done = false;
    while (!done)
    {
        //handling dropped files in different windows
        bool file_dropping_frame = false;
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        done = true;
                        break;
                }
            }
            //TODO: SDL_DROPFILE doesn't support wide characters :_(
            if (event.type == SDL_DROPFILE) {

                dropped_file_path.emplace_back(event.drop.file);
                SDL_free(event.drop.file);

                file_dropping_frame = true;

            }
        }

        {// mouse position handling for panning
            //store previous mouse position before assigning current
            ImVec2 old_mouse_pos = My_Variables.new_mouse_pos;

            //store current mouse position
            My_Variables.new_mouse_pos.x = ImGui::GetMousePos().x;
            My_Variables.new_mouse_pos.y = ImGui::GetMousePos().y;

            //store offset for mouse movement between frames
            My_Variables.mouse_delta.x = My_Variables.new_mouse_pos.x - old_mouse_pos.x;
            My_Variables.mouse_delta.y = My_Variables.new_mouse_pos.y - old_mouse_pos.y;

        }

        {// Store these variables at frame start for cycling palette colors and animations
            My_Variables.CurrentTime_ms = nano_time() / 1'000'000;
            My_Variables.Palette_Update = false;
        }

        //end of event handling/////////////////////////////////////////////////////////////////////////

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        bool show_demo_window = false;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        //ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
        ImGuiID dock_id_right = 0;
        ShowMainMenuBar(&counter, &My_Variables);

        if (firstframe) {
            firstframe = false;
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

            ImGuiID dock_main_id  = dockspace_id; // This variable will track the docking node.
            ImGuiID dock_id_left  = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left,  0.35f, NULL, &dock_main_id);
            ImGuiID dock_id_bleft = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down,  0.64f, NULL, &dock_id_left);
            ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.50f, NULL, &dock_main_id);

            ImGui::DockBuilderDockWindow("###file"      , dock_id_left);
            ImGui::DockBuilderDockWindow("###palette"   , dock_id_bleft);
            ImGui::DockBuilderDockWindow("###preview00" , dock_main_id);
            ImGui::DockBuilderDockWindow("###render00"  , dock_id_right);
            ImGui::DockBuilderDockWindow("###edit00"    , dock_main_id);

            for (int i = 1; i <= 99; i++) {
                char buff1[13];
                sprintf(buff1, "###preview%02d", i);
                char buff2[12];
                sprintf(buff2, "###render%02d", i);
                char buff3[10];
                sprintf(buff3, "###edit%02d", i);

                ImGui::DockBuilderDockWindow(buff1, dock_main_id);
                ImGui::DockBuilderDockWindow(buff2, dock_id_right);
                ImGui::DockBuilderDockWindow(buff3, dock_main_id);
            }
            ImGui::DockBuilderFinish(dockspace_id);
        }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGui::Begin("File Info###file");  // Create a window and append into it.
                //load files
                if (ImGui::Button("Load Files...")) {
                    Open_Files(&usr_info, &counter, My_Variables.pxlFMT_FO_Pal, &My_Variables);
                }
                //used this to create an frm from palette LUT
                // if (ImGui::Button("Save palette animation...")) {
                //     char path_buffer[MAX_PATH];
                //     snprintf(path_buffer, sizeof(path_buffer), "%s%s", usr_info.exe_directory, "/resources/palette/fo_color.pal");
                //     //file management
                //     uint8_t* palette_animation = (uint8_t*)malloc(1024*32);
                //     FILE *File_ptr = fopen(path_buffer, "rb");
                //     fseek(File_ptr, 768, SEEK_SET);
                //     fread(palette_animation, 1024*32, 1, File_ptr);
                //     fclose(File_ptr);
                //     FRM_Header header;
                //     header.version = 4;
                //     header.FPS = 10;
                //     header.Frames_Per_Orient = 32;
                //     header.Frame_Area = 32*32 + sizeof(FRM_Frame);
                //     B_Endian::flip_header_endian(&header);
                //     FRM_Frame frame;
                //     frame.Frame_Height = 32;
                //     frame.Frame_Width  = 32;
                //     frame.Frame_Size   = 32*32;
                //     B_Endian::flip_frame_endian(&frame);
                //     uint8_t* ptr = palette_animation;
                //     snprintf(path_buffer, sizeof(path_buffer), "%s%s", usr_info.exe_directory, "/resources/palette/palette_animation.FRM");
                //     FILE* file = fopen(path_buffer, "wb");
                //     fwrite(&header, sizeof(FRM_Header), 1, file);
                //     for (int i = 0; i < 32; i++)
                //     {
                //         fwrite(&frame, sizeof(FRM_Frame), 1, file);
                //         fwrite(ptr, 1024, 1, file);
                //         ptr += 1024;
                //     }
                //     fclose(file);
                //     free(palette_animation);
                // }

                ImGui::SameLine();
                ImGui::Text("Number Windows = %d", counter);
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

                //contextual buttons for each image slot
                if (My_Variables.window_number_focus >= 0)
                {
                    contextual_buttons(&My_Variables, My_Variables.window_number_focus);
                }

            //set contextual menu for main window
            if (ImGui::IsWindowHovered() && file_dropping_frame) {
                My_Variables.window_number_focus = -1;
                My_Variables.edit_image_focused = false;
            }

            ImGui::End();

            //handle opening dropped files
            if (file_dropping_frame) {
                for (std::string& path : dropped_file_path)
                {
                    std::optional<bool> directory = handle_directory_drop(path.data(),
                                                        My_Variables.F_Prop,
                                                        &My_Variables.window_number_focus,
                                                        &counter,
                                                        &My_Variables.shaders);
        //TODO: maybe I should handle these as an enum instead of std::optional<>
                    if (directory.has_value()) {
                        if (!directory.operator*()) {
                            handle_file_drop(path.data(),
                                &My_Variables.F_Prop[counter],
                                &counter,
                                &My_Variables.shaders);
                        }
                    }
                }
                dropped_file_path.clear();
                file_dropping_frame = false;
            }

            //contextual palette window for MSK vs FRM editing
            if (My_Variables.F_Prop[My_Variables.window_number_focus].edit_MSK) {
                Show_MSK_Palette_Window(&My_Variables);
            }
            else {
                Show_Palette_Window(&My_Variables);
            }
            //update palette at regular intervals
            {
                update_palette_array(My_Variables.shaders.palette,
                                     My_Variables.CurrentTime_ms,
                                    &My_Variables.Palette_Update);
            }

            for (int i = 0; i < counter; i++)
            {
                if (My_Variables.F_Prop[i].file_open_window)
                {
                    Show_Preview_Window(&My_Variables, i, &event);
                }
            }
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    //TODO: test if freeing manually vs freeing by hand is faster/same
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    write_cfg_file(&usr_info, usr_info.exe_directory);

#ifdef QFO2_WINDOWS
    // LocalFree(my_argv);
#endif

    return 0;
}
//end of main////////////////////////////////////////////////////////////////////////

void Show_Preview_Window(struct variables *My_Variables, int counter, SDL_Event* event)
{
    shader_info* shaders = &My_Variables->shaders;

    //shortcuts...possibly replace variables* with just LF*
    LF* F_Prop = &My_Variables->F_Prop[counter];
    SDL_PixelFormat* pxlFMT_FO_Pal = My_Variables->pxlFMT_FO_Pal;

    std::string a = F_Prop->c_name;
    char b[3];
    sprintf(b, "%02d", counter);
    std::string name = a + "###preview" + b;

    // Check image size to match tile size (350x300 pixels)
    bool wrong_size = false;
    ANM_Dir* dir = NULL;
    if (F_Prop->img_data.ANM_dir) {
        dir = &F_Prop->img_data.ANM_dir[F_Prop->img_data.display_orient_num];
        if (dir->num_frames < 2) {
            if (dir->frame_data == NULL) {
                wrong_size = false;
            }
            else {
                wrong_size = ((dir->frame_data->frame_start->w != 350)
                           || (dir->frame_data->frame_start->h != 300));
            }
        }
    }

    if (ImGui::Begin(name.c_str(), (&F_Prop->file_open_window), 0)) {
        //set contextual menu for preview window
        if (ImGui::IsWindowFocused()) {
            My_Variables->window_number_focus = counter;
            My_Variables->edit_image_focused = false;
        }
        ImGui::Checkbox("Show Frame Stats", &F_Prop->show_stats);

        //warn if wrong size for map tile
        if (wrong_size) {
            ImGui::Text("This image is the wrong size to make a tile...");
            ImGui::Text("Size is %dx%d", dir->frame_data->frame_start->w, dir->frame_data->frame_start->h);
            ImGui::Text("Tileable Map images need to be a multiple of 350x300 pixels");
            F_Prop->image_is_tileable = true;
        }
        //TODO: show image name for each frame for new animations
        ImGui::Text(F_Prop->c_name);

        if (F_Prop->img_data.type == FRM) {
            //show the original image for previewing
            //TODO: finish setting up usr.info.show_image_stats in settings config in menu
            Preview_FRM_Image(My_Variables, &F_Prop->img_data, (F_Prop->show_stats || usr_info.show_image_stats));

            //gui video controls
            Gui_Video_Controls(&F_Prop->img_data, F_Prop->img_data.type);
        }
        else if (F_Prop->img_data.type == MSK) {
            Preview_MSK_Image(My_Variables, &F_Prop->img_data, (F_Prop->show_stats || usr_info.show_image_stats));
        }
        else if (F_Prop->img_data.type == OTHER)
        {
            Preview_Image(My_Variables, &F_Prop->img_data, (F_Prop->show_stats || usr_info.show_image_stats));
            //Draw red squares for possible overworld map tiling
            draw_red_squares(&F_Prop->img_data, F_Prop->show_squares);

            draw_red_tiles(&F_Prop->img_data, F_Prop->show_tiles);

            Gui_Video_Controls(&F_Prop->img_data, F_Prop->img_data.type);
        }
        Next_Prev_Buttons(F_Prop, &F_Prop->img_data, shaders);
    }
    ImGui::End();

    // Preview tiles from red boxes
    if (F_Prop->preview_tiles_window) {
        Preview_Tiles_Window(My_Variables, counter);
    }
    // Preview full image
    if (F_Prop->show_image_render) {
        Show_Image_Render(My_Variables, &usr_info, counter);
    }
    // Edit full image
    if (F_Prop->edit_image_window) {
        Edit_Image_Window(My_Variables, &usr_info, counter);
    }
}

void Show_Palette_Window(variables* My_Variables) {

    shader_info* shaders = &My_Variables->shaders;

    bool palette_window = true;
    std::string name = "Default Fallout palette ###palette";
    ImGui::Begin(name.c_str(), &palette_window);

    brush_size_handler(My_Variables);

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {

            int index = y * 16 + x;

            float r = shaders->palette[index*3 + 0];
            float g = shaders->palette[index*3 + 1];
            float b = shaders->palette[index*3 + 2];

            char color_info[12];
            snprintf(color_info, 12, "%d##aa%d", index, index);
            if (ImGui::ColorButton(color_info, ImVec4(r, g, b, 1.0f))) {
                My_Variables->Color_Pick = (uint8_t)(index);
            }

            if (x < 15) ImGui::SameLine();

            if ((index) >= 229) {
                update_palette_array(shaders->palette,
                                     My_Variables->CurrentTime_ms,
                                    &My_Variables->Palette_Update);
            }
        }
    }

    ImGui::End();
}

void Show_MSK_Palette_Window(variables* My_Variables)
{
    bool MSK_palette = true;
    std::string name = "MSK colors ###palette";
    ImGui::Begin(name.c_str(), &MSK_palette);

    brush_size_handler(My_Variables);

    ImGui::Text("Erase Mask                    Draw Mask");
    if (ImGui::ColorButton("Erase Mask", ImVec4(0, 0, 0, 1.0f), 0, ImVec2(200.0f, 200.0f))) {
        My_Variables->Color_Pick = (0);
    }
    ImGui::SameLine();
    if (ImGui::ColorButton("Mark Mask", ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0, ImVec2(200.0f, 200.0f))) {
        My_Variables->Color_Pick = (1);
    }

    ImGui::End();
}

void Preview_Tiles_Window(variables* My_Variables, int counter)
{
    LF* F_Prop = &My_Variables->F_Prop[counter];
    std::string image_name = F_Prop->c_name;
    char window_id[3];
    sprintf(window_id, "%02d", counter);
    std::string name = image_name + " Preview...###render" + window_id;

    //shortcuts

    if (ImGui::Begin(name.c_str(), &F_Prop->preview_tiles_window, 0)) {

        if (ImGui::IsWindowFocused()) {
            My_Variables->window_number_focus = counter;
            My_Variables->tile_window_focused = true;
            My_Variables->render_wind_focused = false;
        }

        if (F_Prop->show_squares) {
            Prev_WMAP_Tiles(My_Variables, &F_Prop->edit_data);
        }
        else {
            Prev_TMAP_Tiles(&usr_info, My_Variables, &F_Prop->edit_data);
        }
    }
    ImGui::End();
}

void Show_Image_Render(variables* My_Variables, struct user_info* usr_info, int counter)
{
    LF* F_Prop = &My_Variables->F_Prop[counter];
    char b[3];
    sprintf(b, "%02d", counter);
    std::string a = F_Prop->c_name;
    std::string name = a + "Render Window...###render" + b;


    if (ImGui::Begin(name.c_str(), &F_Prop->show_image_render, 0)) {

        if (ImGui::IsWindowFocused()) {
            My_Variables->window_number_focus = counter;
            My_Variables->render_wind_focused = true;
            My_Variables->tile_window_focused = false;
        }

        ImGui::Checkbox("Show Frame Stats", &F_Prop->show_stats);

        Preview_FRM_Image(My_Variables, &F_Prop->edit_data, (F_Prop->show_stats || usr_info->show_image_stats));

        Gui_Video_Controls(&F_Prop->edit_data, F_Prop->edit_data.type);
    }
    ImGui::End();
}

void Edit_Image_Window(variables *My_Variables, struct user_info* usr_info, int counter)
{
    char b[3];
    sprintf(b, "%02d", counter);
    LF* F_Prop = &My_Variables->F_Prop[counter];
    std::string a = F_Prop->c_name;
    std::string name = a + " Edit Window...###edit" + b;

    if (ImGui::Begin(name.c_str(), &F_Prop->edit_image_window, 0))
    {
        if (ImGui::IsWindowFocused()) {
            My_Variables->window_number_focus = counter;
            My_Variables->edit_image_focused = true;
        }

        Edit_Image(My_Variables, &My_Variables->F_Prop[counter], My_Variables->Palette_Update, &My_Variables->Color_Pick);

    }

    ImGui::End();

    if (!F_Prop->edit_image_window) {
        My_Variables->window_number_focus = -1;
        My_Variables->edit_image_focused = false;
    }
}

//TODO: Need to test wide character support
//TODO: fix the pxlFMT_FO_Pal loading part
void Open_Files(struct user_info* usr_info, int* counter, SDL_PixelFormat* pxlFMT, struct variables* My_Variables) {
    // Assigns image to Load_Files.image and loads palette for the image
    // TODO: image needs to be less than 1 million pixels (1000x1000)
    // to be viewable in Titanium FRM viewer, what's the limit in the game?
    // (limit is greater than 1600x1200 for menus at least - tested on MR)
    LF* F_Prop = &My_Variables->F_Prop[*counter];

    // if (My_Variables->pxlFMT_FO_Pal == NULL)
    // {
    //     printf("Error: Palette not loaded...");
    //     My_Variables->pxlFMT_FO_Pal = loadPalette("file name for palette here...eventually");
    // }

    F_Prop->file_open_window = Load_Files(F_Prop, &F_Prop->img_data, usr_info, &My_Variables->shaders);

    if (My_Variables->F_Prop[*counter].c_name) {
        (*counter)++;
    }
}

static void ShowMainMenuBar(int* counter, struct variables* My_Variables)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("(demo menu)", NULL, false, false);
            if (ImGui::MenuItem("New - Unimplemented yet...")) {
                /*TODO: add a new file option w/blank surfaces*/ }
            if (ImGui::MenuItem("Open", "Ctrl+O")) { 
                Open_Files(&usr_info, counter, My_Variables->pxlFMT_FO_Pal, My_Variables);
            }
            if (ImGui::MenuItem("Default Fallout Path")) {
                Set_Default_Game_Path(&usr_info, My_Variables->exe_directory);
            }
            if (ImGui::MenuItem("Toggle \"Save Full MSK\" warning")) {
                if (usr_info.save_full_MSK_warning) {
                    usr_info.save_full_MSK_warning = false;
                }
                else {
                    usr_info.save_full_MSK_warning = true;
                }
            }
            if (ImGui::MenuItem("Toggle Image Stats")) {
                if (usr_info.show_image_stats) {
                    usr_info.show_image_stats = false;
                    My_Variables->F_Prop[*counter].show_stats = false;
                }
                else {
                    usr_info.show_image_stats = true;
                    My_Variables->F_Prop[*counter].show_stats = true;
                }
            }
            //if (ImGui::BeginMenu("Open Recent")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit - currently unimplemented, work in progress"))
        {   //TODO: implement undo tree
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Config"))
        {
            if (ImGui::MenuItem("Toggle Auto Mode")) {
                if (usr_info.auto_export != 0) {
                    usr_info.auto_export = 0;
                    usr_info.default_game_path[0] = '\0';
                }
                else {
                    usr_info.auto_export = true;
                }
            }
            if (ImGui::MenuItem("Reset ImGui.ini (not yet implemented)")) {
                char buff[MAX_PATH];
                snprintf(buff, MAX_PATH, "%s%s", My_Variables->exe_directory, "/imgui.ini");
                FILE* file_ptr = fopen(buff, "rb");
                if (file_ptr) {
                    fclose(file_ptr);
                    //TODO: delete file? copy default settings as string?
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void contextual_buttons(variables* My_Variables, int window_number_focus)
{
    //shortcuts, need to replace with direct calls?
    LF* F_Prop = &My_Variables->F_Prop[window_number_focus];
    SDL_PixelFormat* pxlFMT_FO_Pal = My_Variables->pxlFMT_FO_Pal;
    //TODO: save as animated image, needs more work
    static bool open_window     = false;
    static bool single_dir      = false;
    static int  save_type       = UNK;
    static image_data* data_ptr = NULL;

    //Edit_Image buttons
    if (My_Variables->edit_image_focused) {
        int width = F_Prop->edit_data.width;
        int height = F_Prop->edit_data.height;
        // ImGui::Text("Zoom: %%%.2f", My_Variables->F_Prop[window_number_focus].edit_data.scale * 100);
        // ImGui::DragFloat("##zoom", &My_Variables->F_Prop[window_number_focus].edit_data.scale);
        //regular edit image window with animated color pallete painting
        if (!F_Prop->edit_MSK) {
            if (ImGui::Button("Clear All Changes...")) {
                int texture_size = width * height;
                uint8_t* clear = (uint8_t*)malloc(texture_size);
                memset(clear, 0, texture_size);
                glBindTexture(GL_TEXTURE_2D, F_Prop->edit_data.PAL_texture);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                    width, height,
                    0, GL_RED, GL_UNSIGNED_BYTE, clear);
                free(clear);
            }
            //TODO: add frame editing functions/frame saving functions
            if (ImGui::Button("Export Image...")) {
                Save_FRM_Image_OpenGL(&F_Prop->edit_data, &usr_info);
            }
            if (ImGui::Button("Save as Map Tiles...")) {
                //Save_FRM_tiles(F_Prop->PAL_Surface, &user_info);
                Save_FRM_Tiles_OpenGL(F_Prop, &usr_info, My_Variables->exe_directory);
            }
            if (ImGui::Button("Save image as Townmap Tiles...")) {
                // save_TMAP_tiles();
                // My_Variables.
            }
            if (ImGui::Button("Edit MSK Layer...")) {
                F_Prop->edit_MSK = true;
            }
            if (ImGui::Button("Create Mask Layer...")) {

                Create_MSK_OpenGL(&F_Prop->edit_data);
                F_Prop->edit_MSK = true;

                draw_PAL_to_framebuffer(My_Variables->shaders.palette,
                                        My_Variables->shaders.render_PAL_shader,
                                       &My_Variables->shaders.giant_triangle,
                                       &F_Prop->edit_data);
            }
            if (ImGui::Button("Load MSK to this slot...")) {
                Load_Files(F_Prop, &F_Prop->edit_data, &usr_info, &My_Variables->shaders);
            }
        }
        //edit mask window
        else {
            if (ImGui::Button("Clear all changes...")) {
                if (F_Prop->edit_data.MSK_data) {
                    glBindTexture(GL_TEXTURE_2D, F_Prop->edit_data.MSK_texture);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                        width, height, 0,
                        GL_RED, GL_UNSIGNED_BYTE, F_Prop->edit_data.MSK_data);
                }
                else {
                    int texture_size = width * height;
                    uint8_t* clear = (uint8_t*)malloc(texture_size);
                    memset(clear, 0, texture_size);
                    glBindTexture(GL_TEXTURE_2D, F_Prop->edit_data.MSK_texture);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                        width, height,
                        0, GL_RED, GL_UNSIGNED_BYTE, clear);
                    free(clear);
                }
            }
            if (ImGui::Button("Export Mask Tiles...")) {
                //export mask tiles
                Save_MSK_Tiles_OpenGL(&F_Prop->edit_data, &usr_info, My_Variables->exe_directory);
            }
            if (ImGui::Button("Load MSK to this slot...")) {
                Load_Files(F_Prop, &F_Prop->edit_data, &usr_info, &My_Variables->shaders);
            }
            if (ImGui::Button("Export Full MSK...")) {
                Save_Full_MSK_OpenGL(&F_Prop->edit_data, &usr_info);
            }
            if (ImGui::Button("Cancel Editing Mask...")) {
                F_Prop->edit_MSK = false;
            }
        }
        //closes both edit windows, doesn't cancel all edits yet
        if (ImGui::Button("Cancel Editing...")) {
            F_Prop->edit_image_window = false;
            F_Prop->edit_MSK = false;
            My_Variables->edit_image_focused = false;
        }
    }
    //Preview_Image buttons
    else if (!My_Variables->edit_image_focused) {
        ImGui::Text("Zoom: %%%.2f", F_Prop->img_data.scale * 100);
        
        ImGui::PushItemWidth(100);
        ImGui::DragFloat("##Zoom", &F_Prop->img_data.scale, 0.1f, 0.0f, 10.0f, "Zoom: %%%.2fx", 0);
        ImGui::PopItemWidth();
        bool alpha_off = checkbox_handler("Alpha Enabled", &F_Prop->alpha);
        const char * items[] = { "SDL Color Matching", "Euclidan Color Matching" };
        ImGui::SameLine();
        ImGui::Combo("##color_match", &My_Variables->SDL_color, items, IM_ARRAYSIZE(items));

        if (F_Prop->img_data.type == OTHER) {
            if (ImGui::Button("Color Match and Edit")) {
                Prep_Image(F_Prop,
                    pxlFMT_FO_Pal,
                    My_Variables->SDL_color,
                    &F_Prop->edit_image_window, alpha_off);
            }
            if (ImGui::Button("Color Match & Preview as Image")) {
                Prep_Image(F_Prop,
                    pxlFMT_FO_Pal,
                    My_Variables->SDL_color,
                    &F_Prop->show_image_render, alpha_off);
                F_Prop->preview_tiles_window = false;
            }
            //TODO: manage some sort of contextual menu for tileable images?
            if (F_Prop->image_is_tileable) {
                //Tileable image Buttons
                if (ImGui::Button("Color Match & Preview Tiles")) {
                    Prep_Image(F_Prop,
                        pxlFMT_FO_Pal,
                        My_Variables->SDL_color,
                        &F_Prop->preview_tiles_window, alpha_off);
                    //TODO: if image already palettized, need to just feed the texture in
                    F_Prop->show_image_render = false;
                }
                checkbox_handler("Show Map Tile boxes", &F_Prop->show_squares);
                ImGui::SameLine();
                checkbox_handler("Show Town Tiles", &F_Prop->show_tiles);

            }
            if (ImGui::Button("Convert Regular Image to MSK")) {
                Convert_SDL_Surface_to_MSK(F_Prop->img_data.ANM_dir->frame_data->frame_start,
                                          &F_Prop->img_data);
                Prep_Image(F_Prop,
                    NULL,
                    My_Variables->SDL_color,
                    &F_Prop->edit_image_window, alpha_off);
                F_Prop->edit_MSK = true;
            }
        }
        else if (F_Prop->img_data.type == FRM) {
            if (ImGui::Button("Edit this FRM")) {
                Prep_Image(F_Prop,
                    pxlFMT_FO_Pal,
                    My_Variables->SDL_color,
                    &F_Prop->edit_image_window, alpha_off);
            }
            if (ImGui::Button("Preview FRM as full image")) {
                Prep_Image(F_Prop,
                    pxlFMT_FO_Pal,
                    My_Variables->SDL_color,
                    &F_Prop->show_image_render, alpha_off);
                F_Prop->preview_tiles_window = false;
            }
            //TODO: manage some sort of contextual menu for tileable images?
            if (F_Prop->image_is_tileable) {
                //Tileable image Buttons
                if (ImGui::Button("Convert FRM to Tiles")) {
                    Prep_Image(F_Prop,
                        pxlFMT_FO_Pal,
                        My_Variables->SDL_color,
                        &F_Prop->preview_tiles_window, alpha_off);
                    //TODO: if image already palettized, need to just feed the texture in
                    F_Prop->show_image_render = false;
                }
            }
            if (ImGui::Button("Convert FRM Image to MSK")) {
                Convert_SDL_Surface_to_MSK(F_Prop->img_data.ANM_dir->frame_data->frame_start,
                                          &F_Prop->img_data);
                Prep_Image(F_Prop,
                    NULL,
                    My_Variables->SDL_color,
                    &F_Prop->edit_image_window, alpha_off);
                F_Prop->edit_MSK = true;
            }
        }
        else if (F_Prop->img_data.type == MSK) {
            if (ImGui::Button("Edit MSK file")) {
                Prep_Image(F_Prop,
                    pxlFMT_FO_Pal,
                    My_Variables->SDL_color,
                    &F_Prop->edit_image_window, alpha_off);
            }
            //TODO: manage some sort of contextual menu for tileable images?
            if (F_Prop->image_is_tileable) {
                //Tileable image Buttons
                if (ImGui::Button("Color Match & Preview Tiles")) {
                    Prep_Image(F_Prop,
                        pxlFMT_FO_Pal,
                        My_Variables->SDL_color,
                        &F_Prop->preview_tiles_window, alpha_off);
                    //TODO: if image already palettized, need to just feed the texture in
                    F_Prop->show_image_render = false;
                }
            }
            if (ImGui::Button("Convert MSK to BMP")) {
            }
        }

        ImGui::Separator();

        if (F_Prop->img_data.type == FRM && F_Prop->img_data.FRM_dir[F_Prop->img_data.display_orient_num].num_frames > 1) {
            if (ImGui::Button("Save as Animation...")) {
                open_window = true;
                data_ptr = &F_Prop->img_data;
            }
        }

        if (F_Prop->img_data.type == OTHER && F_Prop->img_data.ANM_dir[F_Prop->img_data.display_orient_num].num_frames > 1) {
            if (ImGui::Button("Convert Animation to FRM for Editing")) {
                //F_Prop->edit_image_window = Crop_Animation(&F_Prop->img_data);
                F_Prop->show_image_render = Crop_Animation(&F_Prop->img_data, &F_Prop->edit_data, My_Variables->pxlFMT_FO_Pal);
            }
        }
        if (My_Variables->tile_window_focused) {
            if (ImGui::Button("Save as Map Tiles...")) {
                if (strcmp(F_Prop->extension, "FRM") == 0) {
                    Save_IMG_SDL(F_Prop->img_data.ANM_dir->frame_data->frame_start,
                                &usr_info);
                }
                else {
                    Save_FRM_Tiles_OpenGL(F_Prop, &usr_info, My_Variables->exe_directory);
                }
            }
        }
    }




    //render window buttons
    if (My_Variables->render_wind_focused) {

        if (F_Prop->edit_data.type == FRM && F_Prop->edit_data.FRM_hdr->Frames_Per_Orient > 1) {
            if (ImGui::Button("...Save as Animation...")) {
                open_window = true;
                data_ptr = &F_Prop->edit_data;
            }
            if (ImGui::Button("Save as Single Image...")) {
                open_window = true;

                if (save_type == OTHER) {
                    Save_IMG_SDL(F_Prop->img_data.ANM_dir->frame_data->frame_start,
                                &usr_info);
                }
                if (save_type == FRM) {
                    Save_FRM_Image_OpenGL(&F_Prop->edit_data, &usr_info);
                }
            }
        }
    }




    if (open_window) {
        popup_save_menu(&open_window, &save_type, &single_dir);
        if (save_type == OTHER) {
            //TODO: save as GIF
        }
        if (save_type == FRM) {
            Save_FRM_Animation_OpenGL(data_ptr, &usr_info, F_Prop->c_name);
            open_window = false;
            single_dir  = false;
            save_type   = UNK;
            data_ptr    = NULL;
        }
        if (save_type == FRx && !single_dir) {
            Save_FRx_Animation_OpenGL(data_ptr, usr_info.default_save_path, F_Prop->c_name);
            open_window = false;
            single_dir  = false;
            save_type   = UNK;
            data_ptr    = NULL;
        }
        if (save_type == FRx && single_dir) {
            if (data_ptr->FRM_dir[data_ptr->display_orient_num].orientation < 0) {
                tinyfd_messageBox("You done effed up!",
                    "The selected direction has no data.",
                    "ok", "error", 1);
                open_window = false;
                single_dir  = false;
                save_type   = UNK;
                data_ptr    = NULL;
            }
            else {
                bool success = Save_Single_FRx_Animation_OpenGL(data_ptr, F_Prop->c_name, data_ptr->display_orient_num);
                if (!success) {
                        open_window = false;
                        single_dir  = false;
                        save_type   = UNK;
                        data_ptr    = NULL;
                }
            }
            open_window = false;
            single_dir  = false;
            save_type   = UNK;
            data_ptr    = NULL;
        }
    }



}

//TODO: make this menu nicer
void popup_save_menu(bool* open_window, int* save_type, bool* single_dir)
{
    ImGui::Begin("File type?", open_window);
    if (ImGui::Button("Save as FRM...")) {
        *save_type = FRM;
        *open_window = false;
    }
    if (ImGui::Button("Save selected direction as FRx...")) {
        *save_type = FRx;
        *single_dir = true;
        *open_window = false;
    }
    if (ImGui::Button("Save all available directions as FRx...")) {
        *save_type = FRx;
        *single_dir = false;
        *open_window = false;
    }
    if (ImGui::Button("Save as BMP...")) {
        *save_type = OTHER;
        *open_window = false;
    }
    ImGui::End();
}

#ifdef QFO2_WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    //int size = GetCurrentDirectory(0, NULL);
    //char* buffer = (char*)malloc(size*(sizeof(char)));
    //GetCurrentDirectory(size, buffer);

    return main(0, NULL);
}
#endif