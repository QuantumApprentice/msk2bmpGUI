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


// ImGui header files
#include "imgui-docking/imgui.h"
#include "imgui-docking/imgui_impl_sdl.h"
#include "imgui-docking/imgui_impl_opengl3.h"
#include "imgui-docking/imgui_internal.h"

#include <stdio.h>
#include <SDL.h>
#include <SDL_blendmode.h>
#include <Windows.h>
#include <glad/glad.h>

#include <fstream>
#include <sstream>
#include <iostream>

// My header files
#include "Load_Files.h"
#include "FRM_Convert.h"
#include "Save_Files.h"
#include "Image2Texture.h"
#include "Load_Settings.h"
#include "FRM_Animate.h"
#include "Edit_Image.h"
#include "Preview_Image.h"

#include "display_FRM_OpenGL.h"
#include "Palette_Cycle.h"
#include "Image_Render.h"
#include "Preview_Tiles.h"
#include "MSK_Convert.h"

// Our state
struct user_info user_info;

// Function declarations
void Show_Preview_Window(variables *My_Variables, int counter, SDL_Event* event); //, SDL_Renderer* renderer);
void Preview_Tiles_Window(variables *My_Variables, int counter);
void Show_Image_Render(variables *My_Variables, struct user_info* user_info, int counter);
void Edit_Image_Window(variables *My_Variables, struct user_info* user_info, int counter);

void Show_Palette_Window(struct variables *My_Variables);

//void SDL_to_OpenGl(SDL_Surface *surface, GLuint *Optimized_Texture);

static void ShowMainMenuBar(int* counter, struct variables* My_Variables);
void Open_Files(struct user_info* user_info, int* counter, SDL_PixelFormat* pxlFMT, struct variables* My_Variables);
void Set_Default_Path(struct user_info* user_info);

void contextual_buttons(variables* My_Variables, int window_number_focus);
void Show_MSK_Palette_Window(variables* My_Variables);

//struct position mouse_pos_to_texture_coord(struct position pos, float new_zoom, int frame_width, int frame_height, float* bottom_left_pos);
//void zoom(float zoom_level, struct position focus_point, float* old_zoom, float* new_zoom, float* bottom_left_pos);




//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
//    PSTR lpCmdLine, INT nCmdShow)
//{
//    return main(0, NULL);
//}

// Main code
int main(int, char**)
{
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
    SDL_Window* window = SDL_CreateWindow("Q's Crappy Fallout Image Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        printf("Glad Loader failed?..."); // %s", SDL_GetVideoDriver);
        exit(-1);
    }
    else
    {
        printf("Vendor: %s\n", glGetString(GL_VENDOR));
        printf("Renderer: %s\n", glGetString(GL_RENDERER));
        printf("Version: %s\n", glGetString(GL_VERSION));
        printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    }

    SDL_GL_SetSwapInterval(1); // Enable vsync

    //State variables
    struct variables My_Variables = {};
    Load_Config(&user_info);

    //My_Variables.pxlFMT_FO_Pal = loadPalette("file name for palette here");
    bool success = load_palette_to_array(My_Variables.shaders.palette);
    if (!success) { printf("failed to load palette to array\n"); }

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
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    //Shader stuff
    //Init Framebuffer stuff so I can use shaders
    static int counter = 0;

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // used to reset the default layout back to original
    bool firstframe = true;

    // Main loop
    bool done = false;
    while (!done)
    {
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
        }

        {// mouse position handling for panning
            //store previous mouse position before assigning current
            position old_mouse_pos = My_Variables.new_mouse_pos;

            //store current mouse position
            My_Variables.new_mouse_pos.x = ImGui::GetMousePos().x;
            My_Variables.new_mouse_pos.y = ImGui::GetMousePos().y;

            //store offset for mouse movement between frames
            My_Variables.mouse_delta.x = My_Variables.new_mouse_pos.x - old_mouse_pos.x;
            My_Variables.mouse_delta.y = My_Variables.new_mouse_pos.y - old_mouse_pos.y;

        }

        {// Store these variables at frame start for cycling the palette colors
            My_Variables.CurrentTime = clock();
            My_Variables.Palette_Update = false;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        bool show_demo_window = false;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        //ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
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
            if (ImGui::Button("Load Files...")) {
        //load files
                Open_Files(&user_info, &counter, My_Variables.pxlFMT_FO_Pal, &My_Variables);
            }

            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            //contextual buttons for each image slot
            if (My_Variables.window_number_focus >= 0)
            {
                contextual_buttons(&My_Variables, My_Variables.window_number_focus);
            }
            ImGui::End();

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
                                     My_Variables.CurrentTime,
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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    write_cfg_file(&user_info);

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
    bool wrong_size;

    if (F_Prop->IMG_Surface == NULL) {
        wrong_size = NULL;
    }
    else {
        wrong_size = (F_Prop->IMG_Surface->w != 350) ||
                     (F_Prop->IMG_Surface->h != 300);
    }

    if (ImGui::Begin(name.c_str(), (&F_Prop->file_open_window), 0)) {

        //set contextual menu for preview window
        if (ImGui::IsWindowFocused()) {
            My_Variables->window_number_focus = counter;
            My_Variables->edit_image_focused = false;
        }

        // Check image size to match tile size (350x300 pixels)
        if (wrong_size) {
            ImGui::Text("This image is the wrong size to make a tile...");
            ImGui::Text("Size is %dx%d",
                F_Prop->IMG_Surface->w,
                F_Prop->IMG_Surface->h);
            ImGui::Text("Tileable Map images need to be a multiple of 350x300 pixels");
            F_Prop->image_is_tileable = true;

        }

        //// Rotate the palette for animation
            //new openGL version of pallete cycling
        if (My_Variables->Palette_Update) {
            //redraw FRM to framebuffer every time the palette update timer is true
            if (F_Prop->type == FRM) {
                draw_FRM_to_framebuffer(shaders->palette,
                                       &shaders->render_FRM_shader,
                                       &shaders->giant_triangle,
                                       &F_Prop->img_data);
            }
        }
        //new openGL way of redrawing the FRM image to cycle colors?
        ImGui::Text(F_Prop->c_name);

        //show the original image for previewing
        Preview_Image(My_Variables, &F_Prop->img_data);

        // Draw red boxes to indicate where the tiles will be cut from
        float scale = F_Prop->img_data.scale;
        if (wrong_size) {
            ImDrawList *Draw_List = ImGui::GetWindowDrawList();
            //ImVec2 Origin = F_Prop->img_data.corner_pos;
            ImVec2 Origin;
            Origin.x = F_Prop->img_data.offset.x + ImGui::GetItemRectMin().x;
            Origin.y = F_Prop->img_data.offset.y + ImGui::GetItemRectMin().y;

            ImVec2 Top_Left;
            ImVec2 Bottom_Right = { 0, 0 };
            int max_box_x = F_Prop->IMG_Surface->w / 350;
            int max_box_y = F_Prop->IMG_Surface->h / 300;

            for (int i = 0; i < max_box_x; i++)
            {
                for (int j = 0; j < max_box_y; j++)
                {
                    Top_Left.x = Origin.x + (i * 350)*scale;
                    Top_Left.y = Origin.y + (j * 300)*scale;
                    Bottom_Right = { (float)(Top_Left.x + 350 * scale), (float)(Top_Left.y + 300 * scale) };
                    Draw_List->AddRect(Top_Left, Bottom_Right, 0xff0000ff, 0, 0, 5.0f);
                }
            }
        }
    }
    ImGui::End();
    // Preview tiles from red boxes
    if (F_Prop->preview_tiles_window) {
        Preview_Tiles_Window(My_Variables, counter);
    }
    // Preview full image
    if (F_Prop->show_image_render) {
        Show_Image_Render(My_Variables, &user_info, counter);
    }
    // Edit full image
    if (F_Prop->edit_image_window) {
        Edit_Image_Window(My_Variables, &user_info, counter);
    }
}

void Show_Palette_Window(variables *My_Variables) {

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
                                     My_Variables->CurrentTime,
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
    if (ImGui::ColorButton("Erase Mask", ImVec4(0, 0, 0, 1.0f), NULL, ImVec2(200.0f, 200.0f))) {
        My_Variables->Color_Pick = (0);
    }
    ImGui::SameLine();
    if (ImGui::ColorButton("Mark Mask", ImVec4(1.0f, 1.0f, 1.0f, 1.0f), NULL, ImVec2(200.0f, 200.0f))) {
        My_Variables->Color_Pick = (1);
    }

    ImGui::End();
}

void Preview_Tiles_Window(variables *My_Variables, int counter)
{
    std::string a = My_Variables->F_Prop[counter].c_name;
    char b[3];
    sprintf(b, "%02d", counter);
    std::string name = a + " Preview...###render" + b;

    //shortcuts
    LF* F_Prop = &My_Variables->F_Prop[counter];

    if (ImGui::Begin(name.c_str(), &F_Prop->preview_tiles_window, 0)) {

        if (ImGui::Button("Save as Map Tiles...")) {
            if (strcmp(My_Variables->F_Prop[counter].extension, "FRM") == 0) {
                Save_IMG_SDL(My_Variables->F_Prop[counter].IMG_Surface, &user_info);
            }
            else {
                Save_FRM_tiles_SDL(My_Variables->F_Prop[counter].PAL_Surface, &user_info);
            }
        }

        preview_tiles(My_Variables, &F_Prop->edit_data, counter);

    }
    ImGui::End();
}

void Show_Image_Render(variables *My_Variables, struct user_info* user_info, int counter)
{
    char b[3];
    sprintf(b, "%02d", counter);
    std::string a = My_Variables->F_Prop[counter].c_name;
    std::string name = a + " Preview Window...###render" + b;

    LF* F_Prop = &My_Variables->F_Prop[counter];

    if (ImGui::Begin(name.c_str(), &F_Prop->show_image_render, 0)) {

        if (ImGui::Button("Save as Image...")) {
            if (F_Prop->type == FRM) {
                Save_IMG_SDL(F_Prop->IMG_Surface, user_info);
            }
            else {
                //Save_FRM(F_Prop->PAL_Surface, user_info);
                Save_FRM_OpenGL(&F_Prop->edit_data, user_info);
            }
        }
        image_render(My_Variables, &F_Prop->edit_data);

    }
    ImGui::End();
}

void Edit_Image_Window(variables *My_Variables, struct user_info* user_info, int counter)
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

static void ShowMainMenuBar(int* counter, struct variables* My_Variables)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("(demo menu)", NULL, false, false);
            if (ImGui::MenuItem("New - Unimplemented yet...")) {
                /*TODO: add a new file option w/blank surfaces*/ }
            if (ImGui::MenuItem("Open", "Ctrl+O"))
            { 
                Open_Files(&user_info, counter, My_Variables->pxlFMT_FO_Pal, My_Variables);
            }
            if (ImGui::MenuItem("Default Fallout Path"))
            {
                Set_Default_Path(&user_info);
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
        ImGui::EndMainMenuBar();
    }
}

//TODO: Need to test wide character support
void Open_Files(struct user_info* user_info, int* counter, SDL_PixelFormat* pxlFMT, struct variables* My_Variables) {
    // Assigns image to Load_Files.image and loads palette for the image
    // TODO: image needs to be less than 1 million pixels (1000x1000)
    // to be viewable in Titanium FRM viewer, what's the limit in the game?
    LF* F_Prop = &My_Variables->F_Prop[*counter];

    if (My_Variables->pxlFMT_FO_Pal == NULL)
    {
        printf("Error: Palette not loaded...");
        My_Variables->pxlFMT_FO_Pal = loadPalette("file name for palette here...eventually");
    }

    F_Prop->file_open_window = Load_Files(F_Prop, &F_Prop->img_data, user_info, &My_Variables->shaders);

    //if (std::string_view{ My_Variables.F_Prop[*counter].type } == "FRM")
    //if (F_Prop->file_open_window) {
    //    if (F_Prop->type == FRM)
    //    {   //new openGL way to load FRM
    //        //draw_FRM_to_framebuffer(My_Variables->palette,
    //        //    &My_Variables->render_FRM_shader,
    //        //    &My_Variables->giant_triangle,
    //        //    &F_Prop->img_data);
    //    }
    //    else if (F_Prop->type == MSK)
    //    {   //new OpenGL way to load MSK
    //        //draw_MSK_to_framebuffer(My_Variables->palette,
    //        //    &My_Variables->render_FRM_shader,
    //        //    &My_Variables->giant_triangle,
    //        //    &F_Prop->img_data);
    //    }
    //    else {
    //        //Image2Texture(F_Prop->IMG_Surface,
    //        //    &F_Prop->img_data.render_texture,
    //        //    &F_Prop->file_open_window);
    //        //F_Prop->img_data.height = F_Prop->IMG_Surface->h;
    //        //F_Prop->img_data.width  = F_Prop->IMG_Surface->w;
    //    }
        if (My_Variables->F_Prop[*counter].c_name) { (*counter)++; }
    //}
}

void contextual_buttons(variables* My_Variables, int window_number_focus)
{
    //shortcuts, need to replace with direct calls?
    LF* F_Prop = &My_Variables->F_Prop[window_number_focus];
    SDL_PixelFormat* pxlFMT_FO_Pal = My_Variables->pxlFMT_FO_Pal;

    //Edit_Image buttons
    if (My_Variables->edit_image_focused) {
        int width = F_Prop->edit_data.width;
        int height = F_Prop->edit_data.height;

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
            if (ImGui::Button("Export Image...")) {
                Save_FRM_OpenGL(&F_Prop->edit_data, &user_info);
            }
            if (ImGui::Button("Save as Map Tiles...")) {
                //Save_FRM_tiles(F_Prop->PAL_Surface, &user_info);
                Save_FRM_Tiles_OpenGL(F_Prop, &user_info);
            }
            if (ImGui::Button("Edit MSK Layer...")) {
                F_Prop->edit_MSK = true;
            }
            if (ImGui::Button("Create Mask Layer...")) {

                Create_MSK_OpenGL(&F_Prop->edit_data);
                F_Prop->edit_MSK = true;

                draw_PAL_to_framebuffer(My_Variables->shaders.palette,
                                       &My_Variables->shaders.render_PAL_shader,
                                       &My_Variables->shaders.giant_triangle,
                                       &F_Prop->edit_data);
            }
            if (ImGui::Button("Load MSK to this slot...")) {

                Load_Files(F_Prop, &F_Prop->edit_data, &user_info, &My_Variables->shaders);
                Prep_Image(F_Prop, pxlFMT_FO_Pal, true, &F_Prop->edit_image_window, false);
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
                Save_MSK_Tiles_OpenGL(&F_Prop->edit_data, &user_info);
            }
            if (ImGui::Button("Load MSK to this slot...")) {

                Load_Files(F_Prop, &F_Prop->edit_data, &user_info, &My_Variables->shaders);
                Prep_Image(F_Prop, pxlFMT_FO_Pal, true, &F_Prop->edit_image_window, false);
                draw_PAL_to_framebuffer(My_Variables->shaders.palette,
                                       &My_Variables->shaders.render_PAL_shader,
                                       &My_Variables->shaders.giant_triangle,
                                       &F_Prop->edit_data);
            }
            if (ImGui::Button("Export Full MSK...")) {
                Save_Full_MSK_OpenGL(&F_Prop->edit_data, &user_info);
            }
            if (ImGui::Button("Cancel Editing Mask...")) {
                F_Prop->edit_MSK = false;
            }
        }
        //closes edit window, doesn't cancel all edits yet
        if (ImGui::Button("Cancel Editing...")) {
            F_Prop->edit_image_window = false;
            F_Prop->edit_MSK = false;
        }
    }
    //Preview_Image buttons
    else if (!My_Variables->edit_image_focused) {
        if (ImGui::Button("SDL Convert and Paint")) {
            Prep_Image(F_Prop,
                pxlFMT_FO_Pal,
                true,
                &F_Prop->edit_image_window);
        }
        if (ImGui::Button("Euclidian Convert and Paint")) {
            Prep_Image(F_Prop,
                pxlFMT_FO_Pal,
                false,
                &F_Prop->edit_image_window);
        }
        if (ImGui::Button("Preview as Image - SDL color match")) {
            Prep_Image(F_Prop,
                pxlFMT_FO_Pal,
                true,
                &F_Prop->show_image_render);
        }
        if (ImGui::Button("Preview as Image - SDL color match - Disable Alpha")) {
            bool disable_alpha = true;
            Prep_Image(F_Prop,
                pxlFMT_FO_Pal,
                true,
                &F_Prop->show_image_render, disable_alpha);
        }
        if (ImGui::Button("Preview as Image - Euclidian color match")) {
            Prep_Image(F_Prop,
                pxlFMT_FO_Pal,
                false,
                &F_Prop->show_image_render);
        }

        //TODO: manage some sort of contextual menu for tileable images?
        if (F_Prop->image_is_tileable) {
            //Tileable image Buttons
            if (ImGui::Button("Preview Tiles - SDL color match")) {
                Prep_Image(F_Prop,
                    pxlFMT_FO_Pal,
                    true,
                    &F_Prop->preview_tiles_window);
            }
            if (ImGui::Button("Preview Tiles - Euclidian color match")) {
                Prep_Image(F_Prop,
                    pxlFMT_FO_Pal,
                    false,
                    &F_Prop->preview_tiles_window);
            }
        }
    }
    //image_render buttons
    else if (F_Prop->show_image_render) {
        if (ImGui::Button("Disable Alpha")) {
            My_Variables->shaders.palette[0] = 255;
        }
    }
}