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
#include <SDL.h>
#include "Load_Files.h"
#include "FRM_Convert.h"


#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif
	// Our state
struct variables {
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
	ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
	ImVec2 uv_max = ImVec2(1.0f, 1.0f);
	bool Preview_Tiles = false;
	bool Render_Window = false;
	int Render_Width = 0, Render_Height = 0;

	SDL_Color *PaletteColors = nullptr;

	SDL_Surface* Temp_Surface = nullptr;
	SDL_Surface* Final_Render = nullptr;
	//SDL_Texture* temp_Render = nullptr;
	struct LF F_Prop[99] {};
} My_Variables = {};

void Image2Texture(variables* My_Variables, SDL_Renderer* renderer, int counter);
void ShowPreviewWindow(variables *My_Variables, int counter);
void ShowRenderPreview(variables *My_Variables, 
	ImVec2 *Top_Left, ImVec2 *Bottom_Right, ImVec2 *Origin, 
	int *max_box_x, int *max_box_y, int counter);
void Show_Palette_Window(struct variables *My_Variables, int counter);

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



	bool show_demo_window = true;
	bool show_another_window = false;
	bool done = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
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


			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Open File..."))                      // Buttons return true when clicked (most widgets return true when edited/activated)
			{
				
				// Assigns image to Load_Files.image and loads palette for the image
				// TODO: image needs to be less than 1 million pixels (1000x1000)
				// to be viewable in Titanium FRM viewer, what's the limit in the game?
				Load_Files(My_Variables.F_Prop, counter);
				My_Variables.PaletteColors = loadPalette("string"); // My_Variables.F_Prop[counter].c_name);
				Image2Texture(&My_Variables, renderer, counter);

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
					ShowPreviewWindow(&My_Variables, i);
				}
			}
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

void ShowPreviewWindow(struct variables *My_Variables, int counter)
{
	ImGui::Begin(My_Variables->F_Prop[counter].c_name, (&My_Variables->F_Prop[counter].file_open_window), 0);
	// Check image size to match tile size (350x300 pixels)
	bool wrong_size = (My_Variables->F_Prop[counter].texture_width != 350)
		|| (My_Variables->F_Prop[counter].texture_height != 300);
	if (wrong_size) {
		ImGui::Text("This image is the wrong size to make a tile...");
		ImGui::Text("Size is %dx%d", My_Variables->F_Prop[counter].texture_width, 
									My_Variables->F_Prop[counter].texture_height);
		ImGui::Text("It needs to be 350x300 pixels");
		if (ImGui::Button("Preview Tiles"))
		{
			My_Variables->Preview_Tiles = true;
		}
	}
	ImGui::Text(My_Variables->F_Prop[counter].c_name);
	ImGui::Image(
		My_Variables->F_Prop[counter].Optimized_Texture,
		ImVec2((float)My_Variables->F_Prop[counter].texture_width,
		(float)My_Variables->F_Prop[counter].texture_height),
		My_Variables->uv_min,
		My_Variables->uv_max,
		My_Variables->tint_col,
		My_Variables->border_col);

	if (wrong_size) {
		ImDrawList *Draw_List = ImGui::GetWindowDrawList();
		ImVec2 Origin = ImGui::GetItemRectMin();
		ImVec2 Top_Left = Origin;
		ImVec2 Bottom_Right = { 0, 0 };
		int max_box_x = My_Variables->F_Prop[counter].texture_width/ 350;
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
		if (My_Variables->Preview_Tiles) {
			ShowRenderPreview(My_Variables, &Top_Left, &Bottom_Right, 
									&Origin, &max_box_x, &max_box_y, counter);
		}
		// Final render
		if (My_Variables->Render_Window)
		{
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
	ImGui::End();

}

void Show_Palette_Window(variables *My_Variables, int counter) {
	if (My_Variables->Preview_Tiles)
	{
		std::string a = My_Variables->F_Prop[counter].c_name;
		std::string name = a + " #palette";

		ImGui::Begin(name.c_str(), &My_Variables->Preview_Tiles, ImGuiWindowFlags_NoSavedSettings);

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

void ShowRenderPreview(variables *My_Variables,
	ImVec2 *Top_Left, ImVec2 *Bottom_Right, ImVec2 *Origin,
	int *max_box_x, int *max_box_y, int counter)
{
	Show_Palette_Window(My_Variables, counter);

	ImGui::Begin("Preview Window...", &My_Variables->Preview_Tiles, 0);
	if (ImGui::Button("Render as tiles...")) {
		//My_Variables->Preview_Tiles = false;

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
		My_Variables->Final_Render = FRM_Convert(My_Variables->Temp_Surface);
		//Temp_Surface = SDL_ConvertSurfaceFormat(Final_Render, SDL_PIXELFORMAT_RGBA8888, 0);
		//SDL_LockSurface(Final_Render);							
		//memset(Final_Render->pixels, 0, (1400 * 1500));
		//SDL_UnlockSurface(Final_Render);

		SDL_SaveBMP_RW(My_Variables->Temp_Surface, SDL_RWFromFile("temp2.bmp", "wb"), 1);
		SDL_SaveBMP_RW(My_Variables->Final_Render, SDL_RWFromFile("temp3.bmp", "wb"), 1);



		//temp_Render = SDL_CreateTextureFromSurface(renderer, Final_Render);
		//SDL_QueryTexture(temp_Render,
		//	NULL, NULL,
		//	&Render_Width,
		//	&Render_Height);
		//Render_Window = true;
	}

	// Window to show the tiles split up already
	if (My_Variables->Preview_Tiles) {
		Top_Left = Origin;
		for (int y = 0; y < *max_box_y; y++)
		{
			for (int x = 0; x < *max_box_x; x++)
			{
				Top_Left->x = ((x * 350.0f)) / My_Variables->F_Prop[counter].texture_width;
				Top_Left->y = ((y * 300.0f)) / My_Variables->F_Prop[counter].texture_height;

				*Bottom_Right = { (Top_Left->x + (350.0f / My_Variables->F_Prop[counter].texture_width)),
								(Top_Left->y + (300.0f / My_Variables->F_Prop[counter].texture_height)) };

				ImGui::SameLine();
				ImGui::Image(
					My_Variables->F_Prop[counter].Optimized_Texture,
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

void Image2Texture(variables* My_Variables, SDL_Renderer* renderer, int counter) {
	if (My_Variables->F_Prop[counter].file_open_window) {
		if (!My_Variables->F_Prop[counter].Optimized_Texture)
		{
			SDL_FreeSurface(My_Variables->Temp_Surface);
			My_Variables->Temp_Surface = SDL_ConvertSurfaceFormat(My_Variables->F_Prop[counter].image, SDL_PIXELFORMAT_RGBA8888, 0);
			My_Variables->F_Prop[counter].Optimized_Texture = SDL_CreateTextureFromSurface(renderer, My_Variables->Temp_Surface);
		}
		if (My_Variables->F_Prop[counter].Optimized_Texture == NULL) {
			printf("Unable to optimize image %s! SDL Error: %s\n",
				My_Variables->F_Prop[counter].Opened_File, SDL_GetError());
			My_Variables->F_Prop[counter].file_open_window = false;
		}
		else
		{
			SDL_QueryTexture(My_Variables->F_Prop[counter].Optimized_Texture,
				NULL, NULL,
				&My_Variables->F_Prop[counter].texture_width,
				&My_Variables->F_Prop[counter].texture_height);
		}
	}
}