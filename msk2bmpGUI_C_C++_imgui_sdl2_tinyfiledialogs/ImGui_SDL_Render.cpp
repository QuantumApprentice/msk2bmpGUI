#include "ImGui_SDL_Render.h"
#include "imgui-docking/imgui_impl_sdlrenderer.h"

// SDL
#include <SDL.h>
#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

// SDL_Renderer data
struct ImGui_ImplSDLRenderer_Data
{
	SDL_Renderer*   SDLRenderer;
	SDL_Texture*    FontTexture;
	ImGui_ImplSDLRenderer_Data() { memset(this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
static ImGui_ImplSDLRenderer_Data* ImGui_ImplSDLRenderer_GetBackendData()
{
	return ImGui::GetCurrentContext() ? (ImGui_ImplSDLRenderer_Data*)ImGui::GetIO().BackendRendererUserData : NULL;
}

// Functions
bool ImGui_ImplSDLRenderer_Init(SDL_Renderer* renderer)
{
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.BackendRendererUserData == NULL && "Already initialized a renderer backend!");
	IM_ASSERT(renderer != NULL && "SDL_Renderer not initialized!");

	// Setup backend capabilities flags
	ImGui_ImplSDLRenderer_Data* bd = IM_NEW(ImGui_ImplSDLRenderer_Data)();
	io.BackendRendererUserData = (void*)bd;
	io.BackendRendererName = "imgui_impl_sdlrenderer";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

	bd->SDLRenderer = renderer;

	return true;
}

static void ImGui_ImplSDLRenderer_SetupRenderState()
{
	ImGui_ImplSDLRenderer_Data* bd = ImGui_ImplSDLRenderer_GetBackendData();

	// Clear out any viewports and cliprect set by the user
	// FIXME: Technically speaking there are lots of other things we could backup/setup/restore during our render process.
	SDL_RenderSetViewport(bd->SDLRenderer, NULL);
	SDL_RenderSetClipRect(bd->SDLRenderer, NULL);
}

void ImGui_ImplSDLRenderer_RenderDrawData(ImDrawData* draw_data)
{
	ImGui_ImplSDLRenderer_Data* bd = ImGui_ImplSDLRenderer_GetBackendData();

	// If there's a scale factor set by the user, use that instead
	// If the user has specified a scale factor to SDL_Renderer already via SDL_RenderSetScale(), SDL will scale whatever we pass
	// to SDL_RenderGeometryRaw() by that scale factor. In that case we don't want to be also scaling it ourselves here.
	float rsx = 1.0f;
	float rsy = 1.0f;
	SDL_RenderGetScale(bd->SDLRenderer, &rsx, &rsy);
	ImVec2 render_scale;
	render_scale.x = (rsx == 1.0f) ? draw_data->FramebufferScale.x : 1.0f;
	render_scale.y = (rsy == 1.0f) ? draw_data->FramebufferScale.y : 1.0f;

	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	int fb_width = (int)(draw_data->DisplaySize.x * render_scale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * render_scale.y);
	if (fb_width == 0 || fb_height == 0)
		return;

	// Backup SDL_Renderer state that will be modified to restore it afterwards
	struct BackupSDLRendererState
	{
		SDL_Rect    Viewport;
		bool        ClipEnabled;
		SDL_Rect    ClipRect;
	};
	BackupSDLRendererState old = {};
	old.ClipEnabled = SDL_RenderIsClipEnabled(bd->SDLRenderer) == SDL_TRUE;
	SDL_RenderGetViewport(bd->SDLRenderer, &old.Viewport);
	SDL_RenderGetClipRect(bd->SDLRenderer, &old.ClipRect);

	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = render_scale;

	// Render command lists
	ImGui_ImplSDLRenderer_SetupRenderState();
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					ImGui_ImplSDLRenderer_SetupRenderState();
				else
					pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// Project scissor/clipping rectangles into framebuffer space
				ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
				ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
				if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
				if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
				if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
				if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
				if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
					continue;

				SDL_Rect r = { (int)(clip_min.x), (int)(clip_min.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y) };
				SDL_RenderSetClipRect(bd->SDLRenderer, &r);

				const float* xy = (const float*)((const char*)(vtx_buffer + pcmd->VtxOffset) + IM_OFFSETOF(ImDrawVert, pos));
				const float* uv = (const float*)((const char*)(vtx_buffer + pcmd->VtxOffset) + IM_OFFSETOF(ImDrawVert, uv));
#if SDL_VERSION_ATLEAST(2,0,19)
				const SDL_Color* color = (const SDL_Color*)((const char*)(vtx_buffer + pcmd->VtxOffset) + IM_OFFSETOF(ImDrawVert, col)); // SDL 2.0.19+
#else
				const int* color = (const int*)((const char*)(vtx_buffer + pcmd->VtxOffset) + IM_OFFSETOF(ImDrawVert, col)); // SDL 2.0.17 and 2.0.18
#endif

				// Bind texture, Draw
				SDL_Texture* tex = (SDL_Texture*)pcmd->GetTexID();
				SDL_RenderGeometryRaw(bd->SDLRenderer, tex,
					xy, (int)sizeof(ImDrawVert),
					color, (int)sizeof(ImDrawVert),
					uv, (int)sizeof(ImDrawVert),
					cmd_list->VtxBuffer.Size - pcmd->VtxOffset,
					idx_buffer + pcmd->IdxOffset, pcmd->ElemCount, sizeof(ImDrawIdx));
			}
		}
	}

	// Restore modified SDL_Renderer state
	SDL_RenderSetViewport(bd->SDLRenderer, &old.Viewport);
	SDL_RenderSetClipRect(bd->SDLRenderer, old.ClipEnabled ? &old.ClipRect : NULL);
}