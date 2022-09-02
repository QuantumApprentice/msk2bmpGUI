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
#include <glad/glad.h>
//#include <SDL_opengl.h>
// My header files
#include <iostream>
#include "Load_Files.h"
#include "FRM_Convert.h"
#include "Save_Files.h"
#include "Image2Texture.h"
#include <fstream>
#include <sstream>
#include "Load_Settings.h"

// Our state
struct variables My_Variables = {};
struct user_info user_info;

// Function declarations
void Show_Preview_Window(variables *My_Variables, int counter); //, SDL_Renderer* renderer);
void ShowRenderWindow(variables *My_Variables,
    ImVec2 *Top_Left, ImVec2 *Bottom_Right, ImVec2 *Origin,
    int *max_box_x, int *max_box_y, int counter);
void Show_Image_Render(variables *My_Variables, struct user_info* user_info, int counter);

void Show_Palette_Window(struct variables *My_Variables, int counter);
void Render_and_Save_FRM(variables *My_Variables, struct user_info* user_info, int counter);
void Render_and_Save_IMG(variables *My_Variables, struct user_info* user_info, int counter);

void SDL_to_OpenGl(SDL_Surface *surface, GLuint *Optimized_Texture);
void Prep_Image(variables* My_Variables, int counter, bool color_match, bool* preview_type);

static void ShowMainMenuBar(int* counter);
void Open_Files(struct user_info* user_info, int* counter);
void Set_Default_Path(struct user_info* user_info);

// Main code
int main(int, char**)
{
    Load_Config(&user_info);

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
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
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
            bool * t = NULL;
            bool r = true;
            t = &r;
            ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
            ImGuiID dock_id_right = 0;
            static int counter = 0;
            ShowMainMenuBar(&counter);

            if (firstframe) {
                firstframe = false;
                ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
                ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

                ImGuiID dock_main_id = dockspace_id; // This variable will track the docking node.
                ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.35f, NULL, &dock_main_id);
                ImGuiID dock_id_bottom_left = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.57f, NULL, &dock_id_left);
                ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.50f, NULL, &dock_main_id);

                ImGui::DockBuilderDockWindow("###file", dock_id_left);
                ImGui::DockBuilderDockWindow("###palette", dock_id_bottom_left);
                ImGui::DockBuilderDockWindow("###preview00", dock_main_id);
                ImGui::DockBuilderDockWindow("###render00", dock_id_right);

                for (int i = 1; i <= 99; i++) {
                    char buff[12];
                    sprintf(buff, "###render%02d", i);
                    char buff2[13];
                    sprintf(buff2, "###preview%02d", i);

                    ImGui::DockBuilderDockWindow(buff, dock_id_right);
                    ImGui::DockBuilderDockWindow(buff2, dock_main_id);
                }
                ImGui::DockBuilderFinish(dockspace_id);
            }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGui::Begin("File Info###file");  // Create a window and append into it.
            if (ImGui::Button("Load Files...")) { 
                Open_Files(&user_info, &counter);
            }

            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();

            My_Variables.PaletteColors = loadPalette("file name for palette here");
            Show_Palette_Window(&My_Variables, counter);

            for (int i = 0; i < counter; i++)
            {
                if (My_Variables.F_Prop[i].file_open_window)
                {
                    Show_Preview_Window(&My_Variables, i);
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

void Show_Preview_Window(struct variables *My_Variables, int counter)
{
    std::string a = My_Variables->F_Prop[counter].c_name;
    char b[3];
    sprintf(b, "%02d", counter);
    std::string name = a + "###preview" + b;

    bool wrong_size = (My_Variables->F_Prop[counter].image->w != 350)
        || (My_Variables->F_Prop[counter].image->h != 300);

    ImGui::Begin(name.c_str(), (&My_Variables->F_Prop[counter].file_open_window), 0);
    // Check image size to match tile size (350x300 pixels)
    if (wrong_size) {
        ImGui::Text("This image is the wrong size to make a tile...");
        ImGui::Text("Size is %dx%d", 
                    My_Variables->F_Prop[counter].image->w,
                    My_Variables->F_Prop[counter].image->h);
        ImGui::Text("It needs to be a multiple of 350x300 pixels");
    //Buttons
        if (ImGui::Button("Preview Tiles - SDL color match")) {
            Prep_Image(My_Variables, counter, true, 
                &My_Variables->F_Prop[counter].preview_tiles_window);
        }
        if (ImGui::Button("Preview Tiles - Euclidian color match")) {
            Prep_Image(My_Variables, counter, false, 
                &My_Variables->F_Prop[counter].preview_tiles_window);
        }
        if (ImGui::Button("Preview as Image - SDL color match")) {
            Prep_Image(My_Variables, counter, true, 
                &My_Variables->F_Prop[counter].preview_image_window);
        }
        if (ImGui::Button("Preview as Image - Euclidian color match")) {
            Prep_Image(My_Variables, counter, false, 
                &My_Variables->F_Prop[counter].preview_image_window);
        }
    }
    ImGui::Text(My_Variables->F_Prop[counter].c_name);
    ImGui::Image(
        (ImTextureID)My_Variables->F_Prop[counter].Optimized_Texture,
        ImVec2((float)My_Variables->F_Prop[counter].image->w,
        (float)My_Variables->F_Prop[counter].image->h),
        My_Variables->uv_min,
        My_Variables->uv_max,
        My_Variables->tint_col,
        My_Variables->border_col);

    if (wrong_size) {
        ImDrawList *Draw_List = ImGui::GetWindowDrawList();
        ImVec2 Origin = ImGui::GetItemRectMin();
        ImVec2 Top_Left = Origin;
        ImVec2 Bottom_Right = { 0, 0 };
        int max_box_x = My_Variables->F_Prop[counter].image->w / 350;
        int max_box_y = My_Variables->F_Prop[counter].image->h / 300;

        // Draw red boxes to indicate where the tiles will be cut from
        for (int i = 0; i < max_box_x; i++)
        {
            for (int j = 0; j < max_box_y; j++)
            {
                Top_Left.x = Origin.x + (i * 350);
                Top_Left.y = Origin.y + (j * 300);
                Bottom_Right = { Top_Left.x + 350, Top_Left.y + 300 };
                Draw_List->AddRect(Top_Left, Bottom_Right, 0xff0000ff, 0, 0, 5.0f);
            }
        }
        // Preview tiles from red boxes
        if (My_Variables->F_Prop[counter].preview_tiles_window) {
            ShowRenderWindow(My_Variables, &Top_Left, &Bottom_Right,
                &Origin, &max_box_x, &max_box_y, counter);
        }
        // Preview full image
        if (My_Variables->F_Prop[counter].preview_image_window) {
            Show_Image_Render(My_Variables, &user_info, counter);
        }
    }
    ImGui::End();
}

void Show_Palette_Window(variables *My_Variables, int counter) {
    //TODO: need to add animated colors (FRM_Animate.cpp)
    bool palette_window = true;
    std::string name = "Default Fallout palette ###palette";
    ImGui::Begin(name.c_str(), &palette_window);

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            SDL_Color color = My_Variables->PaletteColors[y * 16 + x];
            float r = (float)color.r / 255.0f;
            float g = (float)color.g / 255.0f;
            float b = (float)color.b / 255.0f;
            ImGui::ColorButton("##aa", ImVec4(r, g, b, 1.0f));
            if (x < 15) ImGui::SameLine();
        }
    }
    ImGui::End();
}

void ShowRenderWindow(variables *My_Variables,
    ImVec2 *Top_Left, ImVec2 *Bottom_Right, ImVec2 *Origin,
    int *max_box_x, int *max_box_y, int counter)
{
    std::string a = My_Variables->F_Prop[counter].c_name;
    char b[3];
    sprintf(b, "%02d", counter);
    std::string name = a + " Preview...###render" + b;

    ImGui::Begin(name.c_str(), &My_Variables->F_Prop[counter].preview_tiles_window, 0);
    if (ImGui::Button("Save as Map Tiles...")) {
        My_Variables->Render_Window = true;
        if (strcmp(My_Variables->F_Prop[counter].extension, "FRM") == 0)
        {
            Save_IMG(My_Variables->F_Prop[counter].image, &user_info);
        }
        else
        {
            Save_FRM_tiles(My_Variables->F_Prop[counter].Pal_Surface, &user_info);
        }
    }

    // Preview window for tiles already converted to palettized and dithered format
    if (My_Variables->F_Prop[counter].preview_tiles_window) {
        Top_Left = Origin;
        for (int y = 0; y < *max_box_y; y++)
        {
            for (int x = 0; x < *max_box_x; x++)
            {
                Top_Left->x = ((x * 350.0f)) / My_Variables->F_Prop[counter].image->w;
                Top_Left->y = ((y * 300.0f)) / My_Variables->F_Prop[counter].image->h;

                *Bottom_Right = { (Top_Left->x + (350.0f / My_Variables->F_Prop[counter].image->w)),
                                  (Top_Left->y + (300.0f / My_Variables->F_Prop[counter].image->h)) };

                ImGui::Image((ImTextureID)
                    My_Variables->F_Prop[counter].Optimized_Render_Texture,
                    ImVec2(350, 300),
                    *Top_Left,
                    *Bottom_Right,
                    My_Variables->tint_col,
                    My_Variables->border_col);

                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
    }
    ImGui::End();
}

void Show_Image_Render(variables *My_Variables, struct user_info* user_info, int counter)
{
    char b[3];
    sprintf(b, "%02d", counter);
    std::string a = My_Variables->F_Prop[counter].c_name;
    std::string name = a + " Preview Window...###render" + b;

    ImGui::Begin(name.c_str(), &My_Variables->F_Prop[counter].preview_image_window, 0);
    if (ImGui::Button("Save as Image...")) {

        My_Variables->F_Prop[counter].preview_image_window = true;

        if (strcmp(My_Variables->F_Prop[counter].extension, "FRM") == 0)
        {
            Save_IMG(My_Variables->F_Prop[counter].image, user_info);
        }
        else
        {
            Save_FRM(My_Variables->F_Prop[counter].Pal_Surface, user_info);
        }
    }
    ImVec2 Origin = ImGui::GetItemRectMin();
    ImVec2 Top_Left = Origin;
    ImVec2 Bottom_Right = { 0, 0 };
    Bottom_Right = ImVec2(Top_Left.x + (My_Variables->F_Prop[counter].image->w),
        Top_Left.y + (My_Variables->F_Prop[counter].image->h));

    ImGui::Image((ImTextureID)
        My_Variables->F_Prop[counter].Optimized_Render_Texture,
        ImVec2(My_Variables->F_Prop[counter].image->w, My_Variables->F_Prop[counter].image->h));
    ImGui::End();
}

// Final render and save
//void Render_and_Save_IMG(variables *My_Variables, struct user_info* user_info, int counter)
//{
//    if (My_Variables->F_Prop[counter].preview_image_window) {
//        Save_IMG(My_Variables->F_Prop[counter].image, user_info);
//    }
//}
//
//void Render_and_Save_FRM(variables *My_Variables, struct user_info* user_info, int counter)
//{
//    if (My_Variables->F_Prop[counter].preview_tiles_window) {
//        // Saves the full image and does not cut into tiles
//        Save_FRM(My_Variables->F_Prop[counter].Pal_Surface, user_info);
//        //--------------------------------------------------
//    }
//}

void Prep_Image(variables* My_Variables, int counter, bool color_match, bool* preview_type) {
    //Paletize to 8-bit FO pallet, and dithered
    My_Variables->F_Prop[counter].Pal_Surface
        = FRM_Color_Convert(My_Variables->F_Prop[counter].image, color_match);
    //Unpalettize image to new surface for display
    My_Variables->F_Prop[counter].Final_Render
        = Unpalettize_Image(My_Variables->F_Prop[counter].Pal_Surface);
    //Converts unpalettized image to texture for display, sets window bool to true
    Image2Texture(My_Variables->F_Prop[counter].Final_Render,
        &My_Variables->F_Prop[counter].Optimized_Render_Texture,
        preview_type);
}

static void ShowMainMenuBar(int* counter)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("(demo menu)", NULL, false, false);
            if (ImGui::MenuItem("New")) {}
            if (ImGui::MenuItem("Open", "Ctrl+O")) { Open_Files(&user_info, counter); }
            if (ImGui::MenuItem("Default Fallout Path")) { Set_Default_Path(&user_info); }
            //if (ImGui::BeginMenu("Open Recent")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
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

void Open_Files(struct user_info* user_info, int* counter) {
        // Assigns image to Load_Files.image and loads palette for the image
        // TODO: image needs to be less than 1 million pixels (1000x1000)
        // to be viewable in Titanium FRM viewer, what's the limit in the game?
        Load_Files(My_Variables.F_Prop, user_info, *counter);
        if (My_Variables.PaletteColors == NULL)
        {
            My_Variables.PaletteColors = loadPalette("file name for palette here");
        }
        Image2Texture(My_Variables.F_Prop[*counter].image,
            &My_Variables.F_Prop[*counter].Optimized_Texture,
            &My_Variables.F_Prop[*counter].file_open_window);

        if (My_Variables.F_Prop[*counter].c_name) { (*counter)++; }
}

