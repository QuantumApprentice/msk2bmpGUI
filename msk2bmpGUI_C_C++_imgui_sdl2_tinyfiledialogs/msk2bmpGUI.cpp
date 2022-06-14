// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// ImGui header files
#include "imgui-docking/imgui.h"
#include "imgui-docking/imgui_impl_sdl.h"
#include "imgui-docking/imgui_impl_opengl3.h"
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

// Our state
struct variables My_Variables = {};

// Function declarations
void ShowPreviewWindow(variables *My_Variables, int counter); //, SDL_Renderer* renderer);
void ShowRenderWindow(variables *My_Variables,
	ImVec2 *Top_Left, ImVec2 *Bottom_Right, ImVec2 *Origin,
	int *max_box_x, int *max_box_y, int counter);
void Show_Palette_Window(struct variables *My_Variables, int counter);
void Render_and_Save_FRM(variables *My_Variables, int counter);
void Render_and_Save_IMG(variables *My_Variables, int counter);

void SDL_to_OpenGl(SDL_Surface *surface, GLuint *Optimized_Texture);

//void Window_Begin();
//void Window_End();

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
    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		//static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			if (ImGui::Button("Load Files...")) 
			{
				// Assigns image to Load_Files.image and loads palette for the image
				// TODO: image needs to be less than 1 million pixels (1000x1000)
				// to be viewable in Titanium FRM viewer, what's the limit in the game?
				Load_Files(My_Variables.F_Prop, counter);
				if (My_Variables.PaletteColors == NULL)
				{
					My_Variables.PaletteColors = loadPalette("file name for palette here");
				}
				Image2Texture(My_Variables.F_Prop[counter].image, 
					&My_Variables.F_Prop[counter].Optimized_Texture,
					&My_Variables.F_Prop[counter].file_open_window);

				if (My_Variables.F_Prop[counter].c_name) { counter++; }
			}

            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();

			for (int i = 0; i < counter; i++)
			{
				if (My_Variables.F_Prop[i].file_open_window)
				{
					//void Window_Begin();
					ShowPreviewWindow(&My_Variables, i); // , renderer);
					//void Window_End();

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

    return 0;
}

// New OpenGL2 implementation - unimplemented yet
//void Window_Begin()
//{
//	// Start the Dear ImGui frame
//	ImGui_ImplOpenGL2_NewFrame();
//	ImGui_ImplSDL2_NewFrame();
//	ImGui::NewFrame();
//}


// Old SDL implementation -- to delete
//void Window_Begin()
//{
//	ImGui_ImplSDLRenderer_NewFrame();
//	ImGui_ImplSDL2_NewFrame();
//	ImGui::NewFrame();
//}
// Old SDL implementation -- to delete
//void Window_End()
//{
//	ImGuiIO& io = ImGui::GetIO();
//	io.DisplaySize = My_Variables.uv_max;
//	// Rendering
//	ImGui::Render();
//	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
//
//	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
//	{
//		SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
//		SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
//		ImGui::UpdatePlatformWindows();
//		ImGui::RenderPlatformWindowsDefault();
//		SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
//	}
//}

void ShowPreviewWindow(struct variables *My_Variables, int counter) //, SDL_Renderer* renderer)
{
	bool wrong_size = (My_Variables->F_Prop[counter].texture_width != 350)
		|| (My_Variables->F_Prop[counter].texture_height != 300);

	ImGui::Begin(My_Variables->F_Prop[counter].c_name, (&My_Variables->F_Prop[counter].file_open_window), 0);
	// Check image size to match tile size (350x300 pixels)
	if (wrong_size) {
		ImGui::Text("This image is the wrong size to make a tile...");
		ImGui::Text("Size is %dx%d", My_Variables->F_Prop[counter].image->w,
									 My_Variables->F_Prop[counter].image->h);
		ImGui::Text("It needs to be 350x300 pixels");
		if (ImGui::Button("Preview Tiles - SDL color match"))
		{
			My_Variables->F_Prop[counter].Pal_Surface
				= FRM_Color_Convert(My_Variables->F_Prop[counter].image, true);

			My_Variables->F_Prop[counter].Final_Render
				= Display_Palettized_Image(My_Variables->F_Prop[counter].Pal_Surface);

			Image2Texture(My_Variables->F_Prop[counter].Final_Render,
				&My_Variables->F_Prop[counter].Optimized_Render_Texture,
				&My_Variables->F_Prop[counter].preview_tiles_window);

			//--------------------------------------------------------------
			SDL_to_OpenGl(
				My_Variables->F_Prop[counter].Final_Render,
				&My_Variables->F_Prop[counter].Optimized_Render_Texture);
			
			// OpenGL stuff below here
			{
				std::ifstream t("Palette_Shader.vert");
				std::stringstream buffer;
				buffer << t.rdbuf();
				std::string str = buffer.str();
				const char * temp_str = str.c_str();
			
				GLuint vertexShader;
				vertexShader = glCreateShader(GL_VERTEX_SHADER);
				glShaderSource(vertexShader, 1, &temp_str, NULL);
				glCompileShader(vertexShader);
				// Error checking
				int success;
				char infoLog[512];
				glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
				if (!success) {
					glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
					std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
				}
			
				GLuint program = glCreateProgram();
			
				GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
				/*
				uniform vec4 palette;
				in vec2 v_texCoord;
				out vec4 finalcolor;
				void main()
				{ finalcolor=texelFetch(img0,xy,0).r; }
			
				in vec2 a_position;
				in vec2 a_texCoord;
				out vec2 v_texCoord;
				void main()
				{ v_texCoord = a_texCoord; }
				*/
			
				char * source =
					"uniform vec4 palette;						\
				in vec2 v_texCoord;							\
				out vec4 finalcolor;						\
				void main()									\
				{finalcolor = texelFetch(img0, xy, 0).r;}";
			
				glShaderSource(fragmentShader, 1, &source, 0);
				// Error checking
				glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
				if (success == GL_FALSE)
				{
					glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
					std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
				}
			
				glAttachShader(program, vertexShader);
				glAttachShader(program, fragmentShader);
				glLinkProgram(program);
			
				GLint isLinked = 0;
				glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
				if (!isLinked) {
					glGetProgramInfoLog(program, 512, NULL, infoLog);
				}
				glDeleteProgram(program);
				glDeleteShader(vertexShader);
				glDeleteShader(fragmentShader);
			
				glDetachShader(program, vertexShader);
				//glUniform4fv();
			
				//My_Variables->F_Prop[counter].Optimized_Render_Texture
				//	= SDL_CreateTextureFromSurface(renderer, My_Variables->F_Prop[counter].Final_Render);
			}
			//--------------------------------------------------------------

			My_Variables->F_Prop[counter].preview_tiles_window = true;
		}
		if (ImGui::Button("Preview Tiles - Euclidian color match"))
		{
			My_Variables->F_Prop[counter].Pal_Surface
				= FRM_Color_Convert(My_Variables->F_Prop[counter].image, false);

			My_Variables->F_Prop[counter].Final_Render
				= Display_Palettized_Image(My_Variables->F_Prop[counter].Pal_Surface);

			Image2Texture(My_Variables->F_Prop[counter].Final_Render,
				&My_Variables->F_Prop[counter].Optimized_Render_Texture,
				&My_Variables->F_Prop[counter].preview_tiles_window);
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
		int max_box_x = My_Variables->F_Prop[counter].texture_width / 350;
		int max_box_y = My_Variables->F_Prop[counter].texture_height / 300;

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
	}
	ImGui::End();
}

void Show_Palette_Window(variables *My_Variables, int counter) {
	if (My_Variables->F_Prop[0].preview_tiles_window == false) 
	{
		if (My_Variables->F_Prop[counter].preview_tiles_window)
		{
			std::string a = My_Variables->F_Prop[counter].c_name;
			std::string name = a + " #palette";

			ImGui::Begin(name.c_str(), 
				&My_Variables->F_Prop[counter].preview_tiles_window, 
				ImGuiWindowFlags_NoSavedSettings);

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
	}
}

void ShowRenderWindow(variables *My_Variables,
	ImVec2 *Top_Left, ImVec2 *Bottom_Right, ImVec2 *Origin,
	int *max_box_x, int *max_box_y, int counter)
{
	Show_Palette_Window(My_Variables, counter);

	std::string a = My_Variables->F_Prop[counter].c_name;
	std::string name = a + " Preview Window...";

	ImGui::Begin(name.c_str(), &My_Variables->F_Prop[counter].preview_tiles_window, 0);

	if (ImGui::Button("Render and save...")) {
		My_Variables->Render_Window = true;
		if (strcmp(My_Variables->F_Prop[counter].extension, "FRM") == 0)
		{
			Render_and_Save_IMG(My_Variables, counter);
		}
		else
		{
			Render_and_Save_FRM(My_Variables, counter);
		}
	}

	ImGui::Image((ImTextureID)
		My_Variables->F_Prop[counter].Optimized_Render_Texture,
		ImVec2((float)My_Variables->F_Prop[counter].image->w,
			   (float)My_Variables->F_Prop[counter].image->h),
		My_Variables->uv_min,
		My_Variables->uv_max,
		My_Variables->tint_col,
		My_Variables->border_col);
		//ImVec2(350, 300),
		//*Top_Left,
		//*Bottom_Right,
		//My_Variables->tint_col,
		//My_Variables->border_col);

	// Preview window for tiles already converted to palettized format
	if (My_Variables->F_Prop[counter].preview_tiles_window) {
		Top_Left = Origin;
		for (int y = 0; y < *max_box_y; y++)
		{
			for (int x = 0; x < *max_box_x; x++)
			{
				Top_Left->x = ((x * 350.0f)) / My_Variables->F_Prop[counter].texture_width;
				Top_Left->y = ((y * 300.0f)) / My_Variables->F_Prop[counter].texture_height;

				*Bottom_Right = { (Top_Left->x + (350.0f / My_Variables->F_Prop[counter].texture_width)),
								(Top_Left->y + (300.0f / My_Variables->F_Prop[counter].texture_height)) };

				ImGui::Image((ImTextureID)
					My_Variables->F_Prop[counter].Optimized_Render_Texture,
					ImVec2(350, 300),
					*Top_Left,
					*Bottom_Right,
					My_Variables->tint_col,
					My_Variables->border_col);
			}
			ImGui::NewLine();
		}
	}
	ImGui::End();
}

// Final render
void Render_and_Save_IMG(variables *My_Variables, int counter)
{
	if (My_Variables->F_Prop[counter].preview_tiles_window) {
		Save_IMG(My_Variables->F_Prop[counter].image);
	}
}

void Render_and_Save_FRM(variables *My_Variables, int counter)
{
	if (My_Variables->F_Prop[counter].preview_tiles_window) {
		Save_FRM(My_Variables->F_Prop[counter].Pal_Surface);
		//Final_Render = SDL_CreateRGBSurface(NULL, 350, 300, 32, 0, 0, 0, 0);
		//SDL_Rect temp_Rect;
		//temp_Rect.w = 350;
		//temp_Rect.h = 300;
		//temp_Rect.x = 0;
		//temp_Rect.y = 0;

		//SDL_BlitSurface(Temp_Surface,
		//	&temp_Rect,
		//	Final_Render,
		//	&temp_Rect);
		//SDL_SaveBMP(Final_Render, "wrldmp00.bmp");

		//temp_Render = SDL_CreateTextureFromSurface(renderer, Final_Render);
		//My_Variables->F_Prop[counter].Final_Render = FRM_Convert(My_Variables->Temp_Surface);
		//Temp_Surface = SDL_ConvertSurfaceFormat(Final_Render, SDL_PIXELFORMAT_RGBA8888, 0);

	/* SDL_SaveBMP_RW(My_Variables->Temp_Surface, SDL_RWFromFile("temp2.bmp", "wb"), 1); */

		//temp_Render = SDL_CreateTextureFromSurface(renderer, Final_Render);
		//SDL_QueryTexture(temp_Render,
		//	NULL, NULL,
		//	&Render_Width,
		//	&Render_Height);
		//Render_Window = true;
		//SDL_Color* PaletteColors = loadPalette();
		//ImGui::Begin("##palette", 0, ImGuiWindowFlags_NoSavedSettings);
		//for (int y = 0; y < 16; y++) {
		//	for (int x = 0; x < 16; x++) {
		//		SDL_Color color = PaletteColors[y * 16 + x];
		//		float r = (float)color.r / 255.0f;
		//		float g = (float)color.g / 255.0f;
		//		float b = (float)color.b / 255.0f;
		//		ImGui::ColorButton("", ImVec4(r, g, b, 1.0f));
		//		if (x < 15) ImGui::SameLine();
		//	}
		//}
		//ImGui::End();

		//SDL_BlitSurface()
		//ImGui::Begin("Rendering?");

		//ImGui::Image(temp_Render,
		//	ImVec2((float)Render_Width,
		//	(float)Render_Height),
		//	uv_min,
		//	uv_max,
		//	tint_col,
		//	border_col);

		//ImGui::End();
	}
}

