#include <iostream>

#include <sdl.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"
#include "imgui/imgui_impl_opengl3.h"
#include "ImFileDialog.h"
#include <Windows.h>
#include <shobjidl.h> //?


#define MERGECOUNT_(a,b)  a##b
#define FINALLABEL_(a) MERGECOUNT_(Finally_ID_, a)
#define FINALLABEL FINALLABEL_(__LINE__)


#if 0

struct _FINALLY
{
	_FINALLY(std::function<void()> INFN) : FN(INFN) {}
	~_FINALLY() { FN(); }
	std::function<void()> FN;

private:
	_FINALLY(const _FINALLY& INFN) {}
};

#define FINALLY _FINALLY FINALLABEL([&]()->void {
#define FINALLYOVER });

#else

template<typename TY>
struct FINAL_CONTAINER
{
	template<typename TY_1>
	FINAL_CONTAINER(TY_1 fn) : FN(fn) {}

	~FINAL_CONTAINER() { FN(); }

	const TY FN;
};

template<typename FN_TYPE>
auto BuildFinal(FN_TYPE FN) -> FINAL_CONTAINER<decltype(FN)> { return FINAL_CONTAINER<decltype(FN)>(FN); }

#define FINALLY auto FINALLABEL = BuildFinal([&]()->void {
#define FINALLYOVER });

#define FINAL(a) FINALLY a FINALLYOVER

#define EXITSCOPE(a) FINAL(a;)

struct FileDir
{
	char str[256];
	bool Valid = false;
};

FileDir SelectFile()
{
	IFileDialog* pFD = nullptr;
	SUCCEEDED(CoInitialize(nullptr));
	auto HRES = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFD));

	if (SUCCEEDED(HRES))
	{
		DWORD Flags;
		HRES = pFD->GetOptions(&Flags);

		if (SUCCEEDED(HRES))
		{
			pFD->SetOptions(FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM);
		}
		else return FileDir();

		FINALLY{
			if (pFD)
				pFD->Release();
		}FINALLYOVER;

		IShellItem* pItem = nullptr;

		WCHAR CurrentDirectory[256];
		GetCurrentDirectoryW(256, CurrentDirectory);

		HRES = pFD->GetFolder(&pItem);
		if (SUCCEEDED(HRES))
		{
			HRES = SHCreateItemFromParsingName(CurrentDirectory, nullptr, IID_PPV_ARGS(&pItem));
			pFD->SetDefaultFolder(pItem);
		}

		FINALLY{
			if (pItem)
				pItem->Release();
		}FINALLYOVER;

		HRES = pFD->Show(nullptr);
		if (SUCCEEDED(HRES))
		{
			IShellItem* pItem = nullptr;
			HRES = pFD->GetResult(&pItem);
			if (SUCCEEDED(HRES))
			{
				FINALLY{
				if (pItem)
					pItem->Release();
				}FINALLYOVER;

				PWSTR FilePath;
				HRES = pItem->GetDisplayName(SIGDN_FILESYSPATH, &FilePath);
				if (SUCCEEDED(HRES))
				{
					size_t length = wcslen(FilePath);
					FileDir	Dir;
					BOOL DefaultCharUsed = 0;
					CCHAR	DChar = ' ';
					auto res = WideCharToMultiByte(CP_UTF8, 0, FilePath, length, Dir.str, 256, nullptr, nullptr);
					if (!res)
					{
						//IErrorInfo*	INFO = nullptr;
						auto Err = GetLastError();
						std::cout << Err;
					}
					else 
					{
						Dir.str[length] = '\0';
						Dir.Valid = true;
						CoTaskMemFree(FilePath);
						return Dir;
					}
				}
			}
		}
	}
	return FileDir();
}



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
	SDL_Window* window = SDL_CreateWindow("msk2bmpGUI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);

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
				SelectFile();
				counter++;
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
#endif