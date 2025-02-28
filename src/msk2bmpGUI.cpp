//PLaces to post when I get this debugged and working well enough to post:
//https://www.nma-fallout.com/threads/help-with-fallout-map-rendering.201969/?ref=0xceed.com
//https://www.nma-fallout.com/threads/fallout-tile-geometry.160287/
//https://www.nma-fallout.com/threads/fallout-2-map-editor.194536/

//ImGUI File Dialogs
//https://github.com/aiekick/ImGuiFileDialog?tab=readme-ov-file


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
bool show_demo_window = false;

// ImGui header files
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include <MiniSDL.h>

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
#include "ImGui_Warning.h"

//remove
#include "B_Endian.h"

// Our state
user_info usr_info;
static struct dropped_files all_dropped_files = {0};

// Function declarations
void Show_Preview_Window(variables *My_Variables, LF* F_Prop, int counter);
void Preview_Tiles_Window(variables *My_Variables, LF* F_Prop, int counter);
void Show_Image_Render(variables *My_Variables, LF* F_Prop, struct user_info* usr_info, int counter);
void Edit_Image_Window(variables *My_Variables, LF* F_Prop, struct user_info* usr_info, int counter);

void Show_Palette_Window(struct variables *My_Variables);

static void ShowMainMenuBar(int* counter, struct variables* My_Variables);
void Open_Files(struct user_info* usr_info, int* counter, Palette* pxlFMT, struct variables* My_Variables);

void main_window_bttns(variables* My_Variables, int index, int* counter);
void contextual_buttons(variables* My_Variables, int window_number_focus);


void Show_MSK_Palette_Window(variables* My_Variables);
void popup_save_menu(bool* open_window, int* save_type, bool* single_dir);

void dropped_files_callback(GLFWwindow* window, int count, const char** paths);

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

// Main code
int main(int argc, char** argv)
{
    int my_argc;

#ifdef QFO2_WINDOWS
    LPWSTR* my_argv = CommandLineToArgvW(GetCommandLineW(), &my_argc);
#elif defined(QFO2_LINUX)
    char** my_argv = argv;
#endif

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        printf("\nglfwInit() failed\n\n");
        return 1;
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 330 core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr) {return 1;}

    glfwSetKeyCallback(window, key_callback);
    glfwSetDropCallback(window, dropped_files_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        printf("Glad Loader failed?...");
        exit(-1);
    } else {
        printf("Vendor: %s\n",       glGetString(GL_VENDOR));
        printf("Renderer: %s\n",     glGetString(GL_RENDERER));
        printf("Version: %s\n",      glGetString(GL_VERSION));
        printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    }


    //State variables
    struct variables My_Variables = {};
    My_Variables.exe_directory = Program_Directory();
    usr_info.exe_directory     = My_Variables.exe_directory;

    char vbuffer[MAX_PATH];
    char fbuffer[MAX_PATH];
    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources/shaders/passthru_shader.vert");
    snprintf(fbuffer, sizeof(fbuffer), "%s%s", My_Variables.exe_directory, "resources/shaders/render_PAL.frag");
    My_Variables.shaders.render_PAL_shader = new Shader(vbuffer, fbuffer);

    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources/shaders/passthru_shader.vert");
    snprintf(fbuffer, sizeof(fbuffer), "%s%s", My_Variables.exe_directory, "resources/shaders/render_FRM.frag");
    My_Variables.shaders.render_FRM_shader = new Shader(vbuffer, fbuffer);

    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources/shaders/passthru_shader.vert");
    snprintf(fbuffer, sizeof(fbuffer), "%s%s", My_Variables.exe_directory, "resources/shaders/passthru_shader.frag");
    My_Variables.shaders.render_OTHER_shader = new Shader(vbuffer, fbuffer);

    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources/grid-texture.png");
    //TODO: am I using tile_texture_prev or tile_texture_rend anymore?
    load_tile_texture(&My_Variables.tile_texture_prev, vbuffer);
    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources/blue_tile_mask.png");
    load_tile_texture(&My_Variables.tile_texture_rend, vbuffer);

    //TODO: add user input for a user-specified palette
    snprintf(vbuffer, sizeof(vbuffer), "%s%s", My_Variables.exe_directory, "resources/palette/fo_color.pal");
    My_Variables.FO_Palette = load_palette_from_path(vbuffer);
    My_Variables.FO_Palette->num_colors = 228;


    Load_Config(&usr_info, My_Variables.exe_directory);

    bool success = load_palette_to_float_array(My_Variables.shaders.palette, My_Variables.exe_directory);
    if (!success) { printf("failed to load palette to float array for OpenGL\n"); }
    My_Variables.shaders.pal = My_Variables.FO_Palette;

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
    ImGui_ImplGlfw_InitForOpenGL(window, true);
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
    //TODO: need to swap this for a linked list (or a static F_Prop?),
    //      or store current image slot in the window itself
    static int counter = 0;



#ifdef QFO2_WINDOWS
    if (my_argv == NULL) {
        MessageBox(NULL,
            "Something went wrong?",
            "argv is NULL?",
            MB_ABORTRETRYIGNORE);
    }
    else {
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

    // std::vector<std::string> dropped_file_path;
    // Main loop
    // bool done = false;
    // while (!done)
    while (!glfwWindowShouldClose(window))
    {
        //handling dropped files in different windows
        bool file_drop_frame = all_dropped_files.count > 0;

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.

        glfwPollEvents();


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
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window
        //    (Most of the sample code is in ImGui::ShowDemoWindow()!
        //    You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

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

#pragma region buttons
                main_window_bttns(&My_Variables, My_Variables.window_number_focus, &counter);
                //contextual buttons for each image slot
                if (My_Variables.window_number_focus >= 0)
                {
                    contextual_buttons(&My_Variables, My_Variables.window_number_focus);
                }

                //set contextual menu for main window
                //when file is dropped on window
                if (ImGui::IsWindowHovered() && file_drop_frame) {
                    My_Variables.window_number_focus = -1;
                    My_Variables.edit_image_focused = false;
                }
                show_popup_warnings();

            ImGui::End();

            //handle opening dropped files
            if (file_drop_frame) {
                char* path = all_dropped_files.first_path;
                for (int i = 0; i < all_dropped_files.count; i++)
                {
                    // std::optional<bool> directory = handle_directory_drop(path.data(),
                    std::optional<bool> directory = handle_directory_drop(path,
                                                        My_Variables.F_Prop,
                                                        &My_Variables.window_number_focus,
                                                        &counter,
                                                        &My_Variables.shaders);
                    //TODO: maybe I should handle these as an enum
                    //      instead of std::optional<>
                    if (directory.has_value()) {
                        if (!directory.operator*()) {
                            handle_file_drop(path,
                                &My_Variables.F_Prop[counter],
                                &counter,
                                &My_Variables.shaders);
                        }
                    }
                    path += strlen(path)+1;
                }
                free(all_dropped_files.first_path);
                memset(&all_dropped_files, 0, sizeof(dropped_files));
                file_drop_frame = false;
            }

            //contextual palette window for MSK vs FRM editing
            if (My_Variables.F_Prop[My_Variables.window_number_focus].edit_MSK && My_Variables.window_number_focus > -1) {
                Show_MSK_Palette_Window(&My_Variables);
            } else {
                Show_Palette_Window(&My_Variables);
            }

            //update palette at regular intervals
            {
                update_palette_array(My_Variables.shaders.palette,
                                     My_Variables.CurrentTime_ms,
                                    &My_Variables.Palette_Update);
            }

            for (int i = 0; i < counter; i++) {
                if (My_Variables.F_Prop[i].file_open_window) {
                    Show_Preview_Window(&My_Variables, &My_Variables.F_Prop[i], i);
                }
                // Edit full image
                if (My_Variables.F_Prop[i].edit_image_window) {
                    Edit_Image_Window(&My_Variables, &My_Variables.F_Prop[i], &usr_info, counter);
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
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* current_context_backup = glfwGetCurrentContext();

            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(current_context_backup);
        }
        glfwSwapBuffers(window);
    }

    // Cleanup
    //TODO: test if freeing manually vs freeing by hand? is faster/same
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    //write config file when closing
    write_cfg_file(&usr_info, usr_info.exe_directory);

#ifdef QFO2_WINDOWS
    // LocalFree(my_argv);
#endif

    return 0;
}
//end of main////////////////////////////////////////////////////////////////////////

//copies dropped file-paths to
//global dropped_files* all_dropped_files
void dropped_files_callback(GLFWwindow* window, int count, const char** paths)
{
    size_t size = 0;
    //get total length of all strings
    for (int i = 0; i < count; i++)
    {
        size += strlen(paths[i]) +1;
    }

    //all_dropped_files is global
    char* c;
    if (all_dropped_files.count > 0) {
        //if already storing filenames
        c = (char*)realloc(all_dropped_files.first_path,
                           all_dropped_files.total_size + size);
        all_dropped_files.first_path = c;
        c += size;
    } else {
        c = (char*)malloc(size);
        all_dropped_files.first_path = c;
    }

    all_dropped_files.count += count;
    all_dropped_files.total_size += size;
    for (int i = 0; i < count; i++) {
        size_t len = strlen(paths[i]) + 1;
        memcpy(c, paths[i], len);
        c += len;
    }
}


void Show_Preview_Window(struct variables *My_Variables, LF* F_Prop, int counter)
{
    shader_info* shaders      = &My_Variables->shaders;
    Palette* pxlFMT_FO_Pal    = My_Variables->FO_Palette;

    //TODO: store image/editing info in the window itself
    //shortcuts...possibly replace variables* with just LF*
    // F_Prop = &My_Variables->F_Prop[counter];
    // static LF* F_Prop = (LF*)malloc(sizeof(LF));
    // memcpy(F_Prop, &My_Variables->F_Prop[counter], sizeof(LF));
    // strncpy(F_Prop->Frst_File, My_Variables->F_Prop[counter].Frst_File, MAX_PATH);
    // strncpy(F_Prop->Prev_File, My_Variables->F_Prop[counter].Prev_File, MAX_PATH);
    // strncpy(F_Prop->Opened_File, My_Variables->F_Prop[counter].Opened_File, MAX_PATH);
    // strncpy(F_Prop->Next_File, My_Variables->F_Prop[counter].Next_File, MAX_PATH);
    // strncpy(F_Prop->Last_File, My_Variables->F_Prop[counter].Last_File, MAX_PATH);
    // F_Prop->c_name = My_Variables->F_Prop[counter].c_name;
    // F_Prop->extension = My_Variables->F_Prop[counter].extension;
    // F_Prop->show_image_render = My_Variables->F_Prop[counter].show_image_render;
    // F_Prop->img_data = My_Variables->F_Prop[counter].img_data;


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
            } else {
                wrong_size = ((dir->frame_data[0]->w != 350)
                           || (dir->frame_data[0]->h != 300));
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
            ImGui::Text("Size is %dx%d", dir->frame_data[0]->w, dir->frame_data[0]->h);
            ImGui::Text("Tileable Map images need to be a multiple of 350x300 pixels");
            F_Prop->image_is_tileable = true;
        }
        //TODO: show image name for each frame for new animations
        ImGui::Text(F_Prop->c_name);

        if (F_Prop->img_data.type == FRM) {
            //show the original image for previewing
            //TODO: finish setting up usr.info.show_image_stats in settings config in menu
            // Preview_FRM_Image(My_Variables, &F_Prop->img_data, (F_Prop->show_stats || usr_info.show_image_stats));
            preview_FRM_SURFACE(My_Variables, &F_Prop->img_data, (F_Prop->show_stats || usr_info.show_image_stats));

            //gui video controls
            Gui_Video_Controls(&F_Prop->img_data, F_Prop->img_data.type);
        }
        else if (F_Prop->img_data.type == MSK) {
            Preview_MSK_Image(My_Variables, &F_Prop->img_data, (F_Prop->show_stats || usr_info.show_image_stats));
        }
        else if (F_Prop->img_data.type == OTHER) {
            Preview_Image(My_Variables, &F_Prop->img_data, (F_Prop->show_stats || usr_info.show_image_stats));
            //Draw red squares for possible overworld map tiling
            draw_red_squares(&F_Prop->img_data, F_Prop->show_squares);

            draw_red_tiles(&F_Prop->img_data, F_Prop->show_tiles);

            Gui_Video_Controls(&F_Prop->img_data, F_Prop->img_data.type);
        }
        Next_Prev_Buttons(F_Prop, &F_Prop->img_data, shaders);

    }
    show_popup_warnings();
    ImGui::End();

    // Preview tiles from red boxes
    if (F_Prop->preview_tiles_window) {
        Preview_Tiles_Window(My_Variables, F_Prop, counter);
    }
    // Preview full image
    if (F_Prop->show_image_render) {
        Show_Image_Render(My_Variables, F_Prop, &usr_info, counter);
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

void Preview_Tiles_Window(variables* My_Variables, LF* F_Prop, int counter)
{
    // LF* F_Prop = &My_Variables->F_Prop[counter];
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
            // Preview_WMAP_Tiles(My_Variables, &F_Prop->edit_data);
            preview_WMAP_tiles_SURFACE(My_Variables, &F_Prop->edit_data);
        }
        else {
            // Prev_TMAP_Tiles(&usr_info, My_Variables, &F_Prop->edit_data);
            prev_TMAP_tiles_SURFACE(&usr_info, My_Variables, &F_Prop->edit_data);
        }
    }
    ImGui::End();
}

void Show_Image_Render(variables* My_Variables, LF* F_Prop, struct user_info* usr_info, int counter)
{
    // LF* F_Prop = &My_Variables->F_Prop[counter];
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

        // Preview_FRM_Image(My_Variables, &F_Prop->edit_data, (F_Prop->show_stats || usr_info->show_image_stats));
        preview_FRM_SURFACE(My_Variables, &F_Prop->edit_data, (F_Prop->show_stats || usr_info->show_image_stats));


        Gui_Video_Controls(&F_Prop->edit_data, F_Prop->edit_data.type);
    }
    ImGui::End();
}

//TODO: need to add direct MSK file editing
//      probably in a different function?
void init_edit_struct_ANM(Edit_Dir* edit_struct, image_data* edit_data, Palette* palette)
{
    //this is for editing MSK files when loading them solo
    if (!edit_data->ANM_dir) {
        // edit_data->display_orient_num = 0;
        // edit_data->FRM_hdr
        edit_struct[0].edit_frame = (Surface**)malloc(sizeof(Surface*));
        if (!edit_struct[0].edit_frame) {
            //TODO: log out to txt file
            set_popup_warning(
                "[ERROR] init_edit_struct_ANM()"
                "Unable to allocate memory for edit_frame.\n"
            );
            printf("Unable to allocate memory for edit_frame: %d\n", __LINE__);
            return;
        }
        edit_struct[0].edit_frame[0] = Create_8Bit_Surface(edit_data->width, edit_data->height, palette);
        if (!edit_struct[0].edit_frame[0]) {
            free(edit_struct[0].edit_frame);
            //TODO: log out to txt file
            set_popup_warning(
                "[ERROR] init_edit_struct_ANM()"
                "Unable to create 8bit surface.\n"
            );
            printf("Unable to create 8bit surface: %d\n", __LINE__);
            return;
        }
        edit_data->ANM_dir = (ANM_Dir*)malloc(sizeof(ANM_Dir*));
        if (!edit_data->ANM_dir) {
            free(edit_struct[0].edit_frame);
            FreeSurface(edit_struct[0].edit_frame[0]);
            //TODO: log out to txt file
            set_popup_warning(
                "[ERROR] init_edit_struct_ANM()"
                "Unable to create 8bit surface.\n"
            );
            printf("Unable to create 8bit surface: %d\n", __LINE__);
            return;
        }
        edit_data->ANM_dir[0].orientation = NE;

        return;
    }

    for (int dir = 0; dir < 6; dir++) {
        int num_frames = edit_data->ANM_dir[dir].num_frames;
        edit_struct[dir].edit_frame = (Surface**)malloc(num_frames*sizeof(Surface*));
        

        for (int frame = 0; frame < num_frames; frame++) {
            if (edit_data->ANM_dir[dir].frame_data == NULL) {
                break;
            }

            //TODO: maybe this needs to be "edit_data->FRM_dir[0].bounding_box.x1" etc?
            //      doing this might make it easier to edit a frame (maybe fewer crashes?)
            //      but doing this and painting outside the official Frame_Width/_Height would
            //      have to be dealt with by expanding the _Width/_Height whenever this happens
            //      AND give the user some feedback that this is happening
            Surface* src = edit_data->ANM_dir[dir].frame_data[frame];
            Surface* dst = Create_8Bit_Surface(src->w, src->h, palette);

            memcpy(dst->pxls, src->pxls, src->w*src->h);

            edit_struct[dir].edit_frame[frame] = dst;
        }
    }
}

//TODO: delete, replaced with init_edit_struct_ANM()
//TODO: need to add direct MSK file editing
//      probably in a different function?
void init_edit_struct(Edit_Dir* edit_struct, image_data* edit_data, Palette* palette)
{
    //this is for editing MSK files when loading them solo
    if (!edit_data->FRM_dir) {
        // edit_data->display_orient_num = 0;
        // edit_data->FRM_hdr
        edit_struct[0].edit_frame = (Surface**)malloc(sizeof(Surface*));
        edit_struct[0].edit_frame[0] = Create_8Bit_Surface(edit_data->width, edit_data->height, palette);
        edit_data->FRM_dir = (FRM_Dir*)malloc(sizeof(FRM_Dir*));
        edit_data->FRM_dir[0].orientation = NE;

        return;
    }

    for (int dir = 0; dir < 6; dir++) {
        int num_frames = edit_data->FRM_dir[dir].num_frames;
        edit_struct[dir].edit_frame = (Surface**)malloc(num_frames*sizeof(Surface*));

        for (int frame = 0; frame < num_frames; frame++) {
            //create 8bit surface and copy FRM image to it
            if (edit_data->FRM_dir[dir].frame_data == NULL) {
                break;
            }

            //TODO: maybe this needs to be "edit_data->FRM_dir[0].bounding_box.x1" etc?
            //      doing this might make it easier to edit a frame (maybe fewer crashes?)
            //      but doing this and painting outside the official Frame_Width/_Height would
            //      have to be dealt with by expanding the _Width/_Height whenever this happens
            //      AND give the user some feedback that this is happening
            FRM_Frame* src = edit_data->FRM_dir[dir].frame_data[frame];
            Surface* dst = edit_struct[dir].edit_frame[frame];
            int w = src->Frame_Width;
            int h = src->Frame_Height;

            edit_struct[dir].edit_frame[frame] = Create_8Bit_Surface(w, h, palette);
            memcpy(dst->pxls, src->frame_start, w*h);
        }
    }
}

void init_MSK_surface(Surface* edit_MSK_srfc)
{
    //these were both for when the entire struct was being allocated at once
    // edit_MSK_srfc->pxls = (uint8_t*)(&(edit_MSK_srfc->pxls)+1);  //alternate way of assigning ptr
    // edit_MSK_srfc.pxls = (uint8_t*)(edit_MSK_srfc+1);

    //TODO: replace 350*300 with something that works for different sized MSK files?
    edit_MSK_srfc->pxls = (uint8_t*)calloc(1, 350*300);

    if (edit_MSK_srfc->pxls) {
        edit_MSK_srfc->channels = 1;
        edit_MSK_srfc->w        = 350;
        edit_MSK_srfc->h        = 300;
        edit_MSK_srfc->pitch    = 350;

    } else {
        printf("[Error] unable to allocate MSK surface pixels.\n");
        return;
    }
}

//TODO: remove this runOnce variable
//      see TODO where it's called
bool runOnce = true;
void Edit_Image_Window(variables *My_Variables, LF* F_Prop, struct user_info* usr_info, int counter)
{
    char b[3];
    sprintf(b, "%02d", counter);
    std::string a = "";
    if (F_Prop->c_name) {
        std::string a = F_Prop->c_name;
    }
    std::string name = a + " Edit Window...###edit" + b;


    image_data* edit_data = &F_Prop->edit_data;
    static Edit_Dir edit_struct[6];
    if (!edit_struct[0].edit_frame) {// && F_Prop->img_data.type != MSK) {
        // init_edit_struct(edit_struct, edit_data, My_Variables->FO_Palette);
        init_edit_struct_ANM(edit_struct, edit_data, My_Variables->FO_Palette);
    }
    static Surface edit_MSK_srfc;
    if (!edit_MSK_srfc.pxls) {
        init_MSK_surface(&edit_MSK_srfc);
    }
    //TODO: this runOnce is dumb, replace with something not dumb
    //      should probably run when loading MSK to slot
    if (runOnce) {
        if (edit_data->MSK_srfc) {
            runOnce = false;
            memcpy(edit_MSK_srfc.pxls, edit_data->MSK_srfc->pxls, edit_MSK_srfc.w*edit_MSK_srfc.h);
        }
    }

    if (ImGui::Begin(name.c_str(), &F_Prop->edit_image_window, 0))
    {
        ImGui::Checkbox("Show Frame Stats", &F_Prop->show_stats);
        if (F_Prop->show_stats) {
            // show_image_stats_FRM(&F_Prop->edit_data, My_Variables->Font);
            show_image_stats_FRM_SURFACE(&F_Prop->edit_data, My_Variables->Font);
        }

        if (ImGui::IsWindowFocused()) {
            My_Variables->window_number_focus = counter-1;
            My_Variables->edit_image_focused  = true;
        }

        //TODO: check this against image_render()
        ImVec2 img_pos = display_img_ImGUI(My_Variables, edit_data);

        Edit_Image(My_Variables, img_pos,
                    &F_Prop->edit_data, edit_struct,
                    &edit_MSK_srfc, F_Prop->edit_MSK,
                    My_Variables->Palette_Update,
                    &My_Variables->Color_Pick);


        Gui_Video_Controls(&F_Prop->edit_data, F_Prop->edit_data.type);
    }

    show_popup_warnings();

    ImGui::End();

    //stuff that happens when window is closed?
    if (!F_Prop->edit_image_window) {
        free(edit_MSK_srfc.pxls);
        edit_MSK_srfc.pxls = NULL;
        for (int i = 0; i < 6; i++)
        {
            free(edit_struct[i].edit_frame);
            edit_struct[i].edit_frame = NULL;
        }

        My_Variables->window_number_focus = -1;
        My_Variables->edit_image_focused = false;
    }
}

//TODO: Need to test wide character support
//TODO: fix the pxlFMT_FO_Pal loading part
void Open_Files(struct user_info* usr_info, int* counter, Palette* pxlFMT, struct variables* My_Variables) {
    // Assigns image to Load_Files.image and loads palette for the image
    // TODO: image needs to be less than 1 million pixels (1000x1000)
    // to be viewable in Titanium FRM viewer, what's the limit in the game?
    // (limit is greater than 1600x1200 for Hi-Res menus at least - tested on MR f2_res.dat)
    LF* F_Prop = &My_Variables->F_Prop[*counter];
    F_Prop->file_open_window = Load_Files(F_Prop, &F_Prop->img_data, usr_info, &My_Variables->shaders);

    if (My_Variables->F_Prop[*counter].c_name) {
        (*counter)++;
    }
}

static void ShowMainMenuBar(int* counter, struct variables* My_Variables)
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::MenuItem("(demo menu)", NULL, false, false);
            if (ImGui::MenuItem("New (not yet implemented)", "", false, false)) {
                /*TODO: add a new file option w/blank surfaces*/ }
            if (ImGui::MenuItem("Open", "Ctrl+O")) { 
                Open_Files(&usr_info, counter, My_Variables->FO_Palette, My_Variables);
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
            //TODO: implement "Open Recent" menu
            //if (ImGui::BeginMenu("Open Recent")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit - WIP")) {
            //TODO: implement undo tree :: all are disabled for now (..., false, false)
            if (ImGui::MenuItem("(not yet implemented)", "", false, false)) {}
            if (ImGui::MenuItem("Undo",  "CTRL+Z", false, false)) {}
            if (ImGui::MenuItem("Redo",  "CTRL+Y", false, false)) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Cut",   "CTRL+X", false, false)) {}
            if (ImGui::MenuItem("Copy",  "CTRL+C", false, false)) {}
            if (ImGui::MenuItem("Paste", "CTRL+V", false, false)) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Config")) {
            if (ImGui::MenuItem("Toggle Auto Mode")) {
                if (usr_info.auto_export != 0) {
                    usr_info.auto_export = 0;
                    usr_info.default_game_path[0] = '\0';
                }
                else {
                    usr_info.auto_export = true;
                }
            }
            if (ImGui::MenuItem("Reset ImGui.ini (not yet implemented)", "", false, false)) {
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
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Shortcuts -- WIP","",false,false)) {

            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void main_window_bttns(variables* My_Variables, int index, int* counter)
{
    if (index < 0) {
        index = 0;
    }
    LF* F_Prop   = &My_Variables->F_Prop[index];
    Palette* pal =  My_Variables->FO_Palette;
    static bool open_window     = false;
    static bool single_dir      = false;
    static int  save_type       = UNK;
    static image_data* src      = &F_Prop->img_data;

    if (ImGui::Button("Load Files...")) {
        Open_Files(&usr_info, counter, pal, My_Variables);
        // if (My_Variables->F_Prop[*counter].c_name) {
        //     (*counter)++;
        // }
    }
    if (ImGui::Button("Open Edit Window")) {
        F_Prop->edit_image_window = true;
        (*counter)++;
    }

    static bool disabled = true;
    if (disabled) ImGui::BeginDisabled();
    if (ImGui::Button("Save")) {

    }
    if (disabled) ImGui::EndDisabled();
}

void contextual_buttons(variables* My_Variables, int window_number_focus)
{
    //shortcuts, need to replace with direct calls?
    LF* F_Prop = &My_Variables->F_Prop[window_number_focus];
    Palette* pxlFMT_FO_Pal = My_Variables->FO_Palette;
    //TODO: save as animated image, needs more work
    //      specifically need to save as GIF at least
    static bool open_window     = false;
    static bool single_dir_b      = false;
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
            //TODO: add frame editing functions/frame saving functions
            // if (ImGui::Button("Export Image...")) {
                Save_Info sv_info;
                sv_info.s_type = all_dirs;
                char* temp = save_FRM_SURFACE(&F_Prop->edit_data, &usr_info, &sv_info);
                if (temp) {
                    F_Prop->c_name = temp;
                }
                
                // Save_FRM_Image_OpenGL(&F_Prop->edit_data, &usr_info);
            // }
            if (ImGui::Button("Save as Overworld Map Tiles...")) {
                //Save_FRM_tiles(F_Prop->PAL_Surface, &user_info);
                Save_FRM_Tiles_OpenGL(F_Prop, &usr_info, My_Variables->exe_directory);
            }
            if (ImGui::Button("Save image as Town Map Tiles...(not yet implemented)")) {
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
                // if (F_Prop->edit_data.MSK_data) {
                //     glBindTexture(GL_TEXTURE_2D, F_Prop->edit_data.MSK_texture);
                //     glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                //         width, height, 0,
                //         GL_RED, GL_UNSIGNED_BYTE, F_Prop->edit_data.MSK_data);
                // }
                // else {
                //     int texture_size = width * height;
                //     uint8_t* clear = (uint8_t*)malloc(texture_size);
                //     memset(clear, 0, texture_size);
                //     glBindTexture(GL_TEXTURE_2D, F_Prop->edit_data.MSK_texture);
                //     glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                //         width, height,
                //         0, GL_RED, GL_UNSIGNED_BYTE, clear);
                //     free(clear);
                // }
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
            F_Prop->edit_MSK = false;
            F_Prop->edit_image_window = false;
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
        const char * items[] = { "Euclidan Color Matching", "Not Implemented..." };
        ImGui::SameLine();
        ImGui::Combo("##color_match", &My_Variables->color_match_algo, items, IM_ARRAYSIZE(items));

        if (F_Prop->img_data.type == OTHER) {
            if (ImGui::Button("Color Match and Edit")) {
                prep_image_SURFACE(
                    F_Prop,
                    pxlFMT_FO_Pal,
                    My_Variables->color_match_algo,
                    &F_Prop->edit_image_window, alpha_off
                );
            }
            if (ImGui::Button("Color Match & Preview as Image")) {
                prep_image_SURFACE(
                    F_Prop,
                    pxlFMT_FO_Pal,
                    My_Variables->color_match_algo,
                    &F_Prop->show_image_render, alpha_off
                );
                // Prep_Image(F_Prop,
                //     pxlFMT_FO_Pal,
                //     My_Variables->color_match_algo,
                //     &F_Prop->show_image_render, alpha_off);
                F_Prop->preview_tiles_window = false;
            }
            //TODO: manage some sort of contextual menu for tileable images?
            if (F_Prop->image_is_tileable) {
                //Tileable image Buttons
                if (ImGui::Button("Color Match & Preview Tiles")) {
                    prep_image_SURFACE(
                        F_Prop,
                        pxlFMT_FO_Pal,
                        My_Variables->color_match_algo,
                        &F_Prop->preview_tiles_window, alpha_off
                    );
                    // Prep_Image(F_Prop,
                    //     pxlFMT_FO_Pal,
                    //     My_Variables->color_match_algo,
                    //     &F_Prop->preview_tiles_window, alpha_off);
                    //TODO: if image already palettized, need to just feed the texture in
                    F_Prop->show_image_render = false;
                }
                checkbox_handler("Show Map Tile boxes", &F_Prop->show_squares);
                ImGui::SameLine();
                checkbox_handler("Show Town Tiles", &F_Prop->show_tiles);

            }
            if (ImGui::Button("Convert Regular Image to MSK")) {
                Convert_SURFACE_to_MSK(F_Prop->img_data.ANM_dir[0].frame_data[0],
                                          &F_Prop->img_data);
                prep_image_SURFACE(
                    F_Prop,
                    pxlFMT_FO_Pal,
                    My_Variables->color_match_algo,
                    &F_Prop->edit_image_window, alpha_off
                );
                // Prep_Image(F_Prop,
                //     NULL,
                //     My_Variables->color_match_algo,
                //     &F_Prop->edit_image_window, alpha_off);
                // F_Prop->edit_MSK = true;
            }
        }
        else if (F_Prop->img_data.type == FRM) {
            if (ImGui::Button("Edit this FRM")) {
                prep_image_SURFACE(
                    F_Prop,
                    pxlFMT_FO_Pal,
                    My_Variables->color_match_algo,
                    &F_Prop->edit_image_window, alpha_off
                );
            }
            if (ImGui::Button("Preview FRM as image (not tiles)")) {
                prep_image_SURFACE(
                    F_Prop,
                    pxlFMT_FO_Pal,
                    My_Variables->color_match_algo,
                    &F_Prop->preview_tiles_window, alpha_off
                );
                // Prep_Image(F_Prop,
                //     pxlFMT_FO_Pal,
                //     My_Variables->color_match_algo,
                //     &F_Prop->show_image_render, alpha_off);
                // F_Prop->preview_tiles_window = false;
            }
            //TODO: manage some sort of contextual menu for tileable images?
            if (F_Prop->image_is_tileable) {
                //Tileable image Buttons
                if (ImGui::Button("Convert FRM to Tiles")) {
                    prep_image_SURFACE(
                        F_Prop,
                        pxlFMT_FO_Pal,
                        My_Variables->color_match_algo,
                        &F_Prop->preview_tiles_window, alpha_off
                    );
                    // Prep_Image(F_Prop,
                    //     pxlFMT_FO_Pal,
                    //     My_Variables->color_match_algo,
                    //     &F_Prop->preview_tiles_window, alpha_off);
                    //TODO: if image already palettized, need to just feed the texture in
                    F_Prop->show_image_render = false;
                }
            }
            if (ImGui::Button("Convert FRM Image to MSK")) {
                Convert_SURFACE_to_MSK(F_Prop->img_data.ANM_dir[0].frame_data[0],
                                          &F_Prop->img_data);
                prep_image_SURFACE(
                    F_Prop,
                    pxlFMT_FO_Pal,
                    My_Variables->color_match_algo,
                    &F_Prop->preview_tiles_window, alpha_off
                );
                // Prep_Image(F_Prop,
                //     NULL,
                //     My_Variables->color_match_algo,
                //     &F_Prop->edit_image_window, alpha_off);
                F_Prop->edit_MSK = true;
            }
        }
        else if (F_Prop->img_data.type == MSK) {
            if (ImGui::Button("Edit MSK file")) {
                prep_image_SURFACE(
                    F_Prop,
                    pxlFMT_FO_Pal,
                    My_Variables->color_match_algo,
                    &F_Prop->edit_image_window, alpha_off
                );
                F_Prop->edit_MSK = true;
            }
            //TODO: manage some sort of contextual menu for tileable images?
            if (F_Prop->image_is_tileable) {
                //Tileable image Buttons
                if (ImGui::Button("Color Match & Preview Tiles")) {
                    prep_image_SURFACE(
                        F_Prop,
                        pxlFMT_FO_Pal,
                        My_Variables->color_match_algo,
                        &F_Prop->preview_tiles_window, alpha_off
                    );
                    // Prep_Image(F_Prop,
                    //     pxlFMT_FO_Pal,
                    //     My_Variables->color_match_algo,
                    //     &F_Prop->preview_tiles_window, alpha_off);
                    //TODO: if image already palettized, need to just feed the texture in
                    F_Prop->show_image_render = false;
                }
            }
            if (ImGui::Button("Convert MSK to BMP")) {
            }
        }

        ImGui::Separator();

        // if (F_Prop->img_data.type == FRM && F_Prop->img_data.FRM_dir[F_Prop->img_data.display_orient_num].num_frames > 1) {
        if (F_Prop->img_data.type == FRM && F_Prop->img_data.ANM_dir[F_Prop->img_data.display_orient_num].num_frames > 1) {
            if (ImGui::Button("Save as Animation...")) {
                open_window = true;
                data_ptr = &F_Prop->img_data;
            }
        }

        if (F_Prop->img_data.type == OTHER && F_Prop->img_data.ANM_dir[F_Prop->img_data.display_orient_num].num_frames > 1) {
            if (ImGui::Button("Convert Animation to FRM for Editing")) {
                // F_Prop->edit_image_window = Crop_Animation(&F_Prop->img_data);
                // F_Prop->show_image_render = Crop_Animation(&F_Prop->img_data, &F_Prop->edit_data, My_Variables->FO_Palette);
                F_Prop->show_image_render = crop_animation_SURFACE(&F_Prop->img_data, &F_Prop->edit_data, My_Variables->FO_Palette, 0, &My_Variables->shaders);
            }
        }
        if (My_Variables->tile_window_focused) {
            if (ImGui::Button("Save as Map Tiles...")) {
                if (strcmp(F_Prop->extension, "FRM") == 0) {
                    Save_IMG_STB(F_Prop->img_data.ANM_dir[0].frame_data[0],
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
                    Save_IMG_STB(F_Prop->img_data.ANM_dir[0].frame_data[0],
                                &usr_info);
                }
                if (save_type == FRM) {
                    Save_FRM_Image_OpenGL(&F_Prop->edit_data, &usr_info);
                }
            }
        }
    }




    if (open_window) {
        popup_save_menu(&open_window, &save_type, &single_dir_b);
        if (save_type == OTHER) {
            //TODO: save as GIF
        }
        if (save_type == FRM) {
            Save_FRM_Animation_OpenGL(data_ptr, &usr_info, F_Prop->c_name);
            open_window = false;
            single_dir_b  = false;
            save_type   = UNK;
            data_ptr    = NULL;
        }
        if (save_type == FRx && !single_dir_b) {
            Save_FRx_Animation_OpenGL(data_ptr, usr_info.default_save_path, F_Prop->c_name);
            open_window = false;
            single_dir_b  = false;
            save_type   = UNK;
            data_ptr    = NULL;
        }
        if (save_type == FRx && single_dir_b) {
            if (data_ptr->FRM_dir[data_ptr->display_orient_num].orientation < 0) {
                tinyfd_messageBox(
                    "You done effed up!",
                    "The selected direction has no data.",
                    "ok", "error", 1);
                open_window = false;
                single_dir_b  = false;
                save_type   = UNK;
                data_ptr    = NULL;
            }
            else {
                bool success = Save_Single_FRx_Animation_OpenGL(data_ptr, F_Prop->c_name, data_ptr->display_orient_num);
                if (!success) {
                        open_window = false;
                        single_dir_b  = false;
                        save_type   = UNK;
                        data_ptr    = NULL;
                }
            }
            open_window = false;
            single_dir_b  = false;
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