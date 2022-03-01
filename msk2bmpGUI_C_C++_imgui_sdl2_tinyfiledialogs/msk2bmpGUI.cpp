#include <iostream>
// Dear ImGui: standalone example application for SDL2 + SDL_Renderer
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// Important to understand: SDL_Renderer is an _optional_ component of SDL. We do not recommend you use SDL_Renderer
// because it provide a rather limited API to the end-user. We provide this backend for the sake of completeness.
// For a multi-platform app consider using e.g. SDL+DirectX on Windows and SDL+OpenGL on Linux/OSX.

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include "tinyfiledialogs.h"
#include <string.h>

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

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

	// Setup window
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);

	// Setup SDL_Renderer instance
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		SDL_Log("Error creating SDL_Renderer!");
		return false;
	}
	//SDL_RendererInfo info;
	//SDL_GetRendererInfo(renderer, &info);
	//SDL_Log("Current SDL_Renderer: %s", info.name);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);

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
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	SDL_Surface* image1 = NULL;
	SDL_Surface* image2 = NULL;
	SDL_Texture* optimizedSurface = NULL;
	SDL_Texture* Final_Render = NULL;
	int texture_width = 0;
	int texture_height = 0;
	char * Opened_File = nullptr;
	char * c_name = nullptr;
	bool file_open_window[1][1] = { false };
	bool Render_Window = false;
	bool Preview_Tiles = false;
	int Render_Width = 0, Render_Height = 0;

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
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		//if (show_demo_window)
		//	ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;
			char * FilterPattern1[2] = { "*.bmp", "*.png" };

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Open File..."))                      // Buttons return true when clicked (most widgets return true when edited/activated)
			{
				Opened_File = tinyfd_openFileDialog(
					"Open files...",
					"",
					2,
					FilterPattern1,
					NULL,
					1);
				counter++;
				if (!Opened_File) {
					/*	tinyfd_messageBox(
						"Error",
						"No file opened...",
						"ok",
						"error",
						0);		*/			
				}
				else {
					c_name = strrchr(Opened_File, '/\\') + 1;
					image1 = IMG_Load(Opened_File);
					if (image1 == NULL)
					{
						printf("Unable to open image file %s! SDL Error: %s\n",
							Opened_File,
							SDL_GetError());
					}
					else
					{//Convert surface to screen format
						optimizedSurface = SDL_CreateTextureFromSurface(renderer, image1);
						
						file_open_window[0][0] = true;
						if (optimizedSurface == NULL) {
							printf("Unable to optimize image %s! SDL Error: %s\n", Opened_File, SDL_GetError());
							file_open_window[1][1] = false;
						}
						else
						{
							SDL_QueryTexture(optimizedSurface,
								NULL, NULL,
								&texture_width,
								&texture_height);
						}
					}					
				}
			}

			ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
			ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
			ImVec2 uv_max = ImVec2(1.0f, 1.0f);

			if (file_open_window[0][0]) {
				ImGui::Begin(c_name, (&file_open_window[1][1]), 0);
				bool wrong_size = (texture_width != 350) 
					|| (texture_height != 300);
				if (wrong_size) {
					ImGui::Text("This image is the wrong size to make a tile...");
					ImGui::Text("Size is %dx%d", texture_width, texture_height);
					ImGui::Text("It needs to be 350x300 pixels");
					if (ImGui::Button("Preview Tiles"))
					{
						Preview_Tiles = true;
					}
				}
				ImGui::Text(c_name);
				ImGui::Image(
					optimizedSurface,
					ImVec2((float)texture_width,
						(float)texture_height),
					uv_min,
					uv_max,
					tint_col,
					border_col);

				// Check image size to match tile size (350x300 pixels)
				if (wrong_size) {
					ImDrawList *Draw_List = ImGui::GetWindowDrawList();
					ImVec2 Origin = ImGui::GetItemRectMin();
					ImVec2 Top_Left = Origin;
					ImVec2 Bottom_Right = { 0, 0 };
					int max_box_x = texture_width / 350;
					int max_box_y = texture_height / 300;
					
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
					if (Preview_Tiles) {
						ImGui::Begin("Preview Window...", &Preview_Tiles, 0);
						
						if(ImGui::Button("Render as tiles...")) {

							//uint8_t* pix = (uint8_t*)(image1->pixels);
							memset(image1->pixels, 0xffff, 255);
							int success;
							Uint32 temp = {0};

							SDL_QueryTexture(Final_Render,
								&temp, NULL,
								&Render_Width,
								&Render_Height);
							//SDL_CreateRGBSurface(0, Render_Width, Render_Height, 32, 0, 0, 0, 0);

							image2 = SDL_ConvertSurfaceFormat(image2, SDL_PIXELFORMAT_RGBA8888, 0);
							Final_Render = SDL_CreateTextureFromSurface(renderer, image2);
							//SDL_UpdateTexture(Final_Render, NULL, image1->pixels, image1->pitch);

							Render_Window = true;
						}
						
						Top_Left = Origin;
						for (int y = 0; y < max_box_y; y++)
						{
							for (int x = 0; x < max_box_x; x++)
							{
								Top_Left.x = ((x * 350.0f))/texture_width;
								Top_Left.y = ((y * 300.0f))/texture_height;

								Bottom_Right = { (Top_Left.x + (350.0f/texture_width)), 
									(Top_Left.y + (300.0f/texture_height)) };

								ImGui::SameLine();
								ImGui::Image(
									optimizedSurface,
									ImVec2(350, 300),
									Top_Left,
									Bottom_Right,
									tint_col,
									border_col);
								
							}
							ImGui::NewLine();

						}
						ImGui::End();
					}
					if (Render_Window)
					{
						ImGui::Begin("Rendering?");

						ImGui::Image(Final_Render,
							ImVec2((float)Render_Width,
							(float)Render_Height),
							uv_min,
							uv_max,
							tint_col,
							border_col);

						ImGui::End();
					}
				}

				ImGui::End();
			}

			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(renderer);
	}

	// Cleanup
	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
