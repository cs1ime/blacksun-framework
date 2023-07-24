#include <SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdlrenderer.h"
#include <stdio.h>
#include <protocol-rend.h>
#include <protocol-universal-input.h>
#include <protocol-universal-shm.h>
#include <protocol-frontend.h>
#include "vdpau/include/vdpau.h"
#include <util.h>
#include <print-conrtol.h>
#include <memory>

namespace UI_VDPAU
{
	extern bool m_guiinited;
}

namespace UI
{
	std::shared_ptr<frontend> m_frontend;

	bool m_ImguiInited = false;
	bool m_showui = false;
	ImVec2 m_newcursor_pos = {0, 0};
	bool m_newcursor_pos_inited = false;

	void mousemove(int x, int y)
	{
		if (m_showui)
			return;
		if (m_ImguiInited)
		{
			SDL_Event event;
			event.type = SDL_MOUSEMOTION;
			event.motion.x = 0;										// 鼠标位置的x坐标
			event.motion.y = 0;										// 鼠标位置的y坐标
			event.motion.xrel = x;								// 鼠标在x方向上移动的距离
			event.motion.yrel = y;								// 鼠标在y方向上移动的距离
			event.motion.state = SDL_BUTTON_LEFT; // 鼠标状态，这里假设左键被按下

			bool r = SDL_PushEvent(&event);
			// printf("%d\n", r);
		}
	}

	void UI_loop()
	{
	}
	bool UI_event(SDL_Event *event)
	{
		// mousemove(10, 10);
		puts("event");
		if (UI::m_ImguiInited || UI_VDPAU::m_guiinited)
		{
			if (event->type == SDL_WINDOWEVENT)
			{
				if (event->window.event == SDL_WINDOWEVENT_RESIZED || event->window.event == SDL_WINDOWEVENT_MINIMIZED)
				{
					exit(1);
				}
			}
		}
		if (UI::m_ImguiInited)
		{

			ImGuiIO &io = ImGui::GetIO();
			m_frontend->update_hotkey(event);
			if (m_frontend->hkey()->IsToggle(SDL_SCANCODE_HOME))
			{
				ImVec2 Center = ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y / 2);

				if (UI::m_newcursor_pos_inited == false)
				{
					UI::m_newcursor_pos = Center;
					UI::m_newcursor_pos_inited = true;
				}

				io.MousePos = UI::m_newcursor_pos;

				UI::m_showui = !UI::m_showui;
				return false;
			}

			// process block keys

			if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP)
			{
				if (m_frontend->is_key_blocked(event->key.keysym.scancode))
				{
					return false;
				}
			}
			if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP)
			{
				if (m_frontend->is_btn_blocked(event->button.button))
				{
					return false;
				}
			}

			if (UI::m_showui)
			{

				if (event->type == SDL_MOUSEMOTION)
				{
					UI::m_newcursor_pos.x += event->motion.xrel;
					UI::m_newcursor_pos.y += event->motion.yrel;
					if (UI::m_newcursor_pos.x >= io.DisplaySize.x)
					{
						UI::m_newcursor_pos.x = io.DisplaySize.x;
					}
					else if (UI::m_newcursor_pos.x < 0)
					{
						UI::m_newcursor_pos.x = 0;
					}
					if (UI::m_newcursor_pos.y >= io.DisplaySize.y)
					{
						UI::m_newcursor_pos.y = io.DisplaySize.y;
					}
					else if (UI::m_newcursor_pos.y < 0)
					{
						UI::m_newcursor_pos.y = 0;
					}

					event->motion.x = UI::m_newcursor_pos.x;
					event->motion.y = UI::m_newcursor_pos.y;
				}

				ImGui_ImplSDL2_ProcessEvent(event);
				return false;
			}
		}
		return true;
	}
}

namespace UI
{
	ImDrawList *g_list = 0;
	void DrawDispatch(TaskElement *ele)
	{
		switch (ele->Type)
		{
		case ELEMENT_LINE:
		{
			auto e = (ElementLine *)&ele->Data;
			g_list->AddLine(ImVec2(e->x1, e->y1), ImVec2(e->x2, e->y2), e->col);

			break;
		}
		case ELEMENT_CIRCLE:
		{
			auto e = (ElementCircle *)&ele->Data;
			// g_rend->Circle({ e->centerx, e->centery }, FColor(e->col), e->rad*2.2f, 1);
			break;
		}
		case ELEMENT_SOILDRECT:
		{
			auto e = (ElementSoildRect *)&ele->Data;
			// g_rend->FillRectangle({ (int)e->x,(int)e->y }, { (int)e->w,(int)e->h }, FColor(e->col));

			break;
		}
		case ELEMENT_STRING:
		{
			// auto e = (ElementString*)&ele->Data;
			// WCHAR data[100];
			// memset(data, 0, sizeof(data));
			// DxUTF8ToUnicode(&e->str, data, 99);
			// g_rend->String(&g_Font, { e->x, e->y }, data, e->col);
			break;
		}
		case ELEMENT_STROKESTRING:
		{
			auto e = (ElementStrokeString *)&ele->Data;
			
			break;
		}
		default:
			break;
		}
	}

	__forceinline void Rendering()
	{
		ImGuiIO &io = ImGui::GetIO();
		ImDrawList *list = ImGui::GetForegroundDrawList();
		UI::g_list = list;
		m_frontend->rend()->UpdateScreenSize((uint32_t)io.DisplaySize.x, (uint32_t)io.DisplaySize.y);
		m_frontend->rend()->ProcessTask(UI::DrawDispatch);

		if (UI::m_showui)
		{

			list->AddRectFilled(ImVec2(0, 0), io.DisplaySize, ImColor(0, 0, 0, 75), 0, 0);

			ImGui::Begin("ccc");
			ImGui::Text("222");
			ImGui::End();

			ImGuiIO &io = ImGui::GetIO();

			list->AddCircleFilled(io.MousePos, 2, ImColor(255, 255, 255));
			list->AddCircle(io.MousePos, 3.5, ImColor(0, 0, 0), 0, 1.5);
		}

		ImGui::Render();
	}

}

namespace UI_EGL
{
	__forceinline void UI_init(SDL_Window *window, SDL_GLContext ctx)
	{
		puts("init");
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplSDL2_InitForOpenGL(window, ctx);
		ImGui_ImplOpenGL3_Init("#version 100");

		// SDL_SetRelativeMouseMode(SDL_TRUE);

		UI::m_ImguiInited = true;
		UI::m_newcursor_pos_inited = false;
	}
	__forceinline void UI_update()
	{
		// puts("update");
		if (UI::m_ImguiInited)
		{

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();

			UI::Rendering();

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
	}
	__forceinline void UI_destory()
	{
		puts("destory");

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		UI::m_ImguiInited = false;
		UI::m_newcursor_pos_inited = false;
	}
}

namespace UI_VDPAU
{
	struct RenderContext
	{
		VdpBitmapSurface m_cachedVdpSurface = 0;
		pthread_mutex_t m_vdpsurface_lock = {0};

		VdpOutputSurfaceRenderBitmapSurface *m_VdpOutputSurfaceRenderBitmapSurface;
		VdpBitmapSurfaceDestroy *m_VdpBitmapSurfaceDestroy;
		VdpBitmapSurfaceCreate *m_VdpBitmapSurfaceCreate;
		VdpBitmapSurfacePutBitsNative *m_VdpBitmapSurfacePutBitsNative;
		VdpDevice m_Device;
	};

	RenderContext m_renderContext = {0};
	pthread_t m_thread = 0;
	bool m_noticeThreadClose = false;
	bool m_guiinited = false;

	void RenderThread(void *argu_ctx)
	{

		int width = 1920;
		int height = 1080;

		RenderContext *ctx = (RenderContext *)argu_ctx;

		SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
		SDL_Window *window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
		SDL_Surface *window_surface = SDL_GetWindowSurface(window);

		SDL_Surface *surface = SDL_ConvertSurfaceFormat(window_surface, SDL_PIXELFORMAT_ARGB8888, 0);

		SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(surface);
		if (renderer == NULL)
		{
			SDL_Log("Error creating SDL_Renderer!");
			return;
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();
		(void)io;

		ImGui::StyleColorsDark();

		ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
		ImGui_ImplSDLRenderer_Init(renderer);

		UI::m_ImguiInited = true;

		// Our state
		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.00f);

		while (1)
		{
			if (ctx->m_Device &&
					ctx->m_VdpBitmapSurfaceCreate && ctx->m_VdpBitmapSurfacePutBitsNative && ctx->m_VdpBitmapSurfaceDestroy)
			{
				break;
			}
			SDL_Delay(20);
		}

		//双缓冲
		//

		VdpBitmapSurface stock_vdp_surface[2];
		int current_use_idx = 0;

		{
			VdpStatus stats = VDP_STATUS_OK;

			stats = ctx->m_VdpBitmapSurfaceCreate(
					ctx->m_Device,
					VDP_RGBA_FORMAT_B8G8R8A8,
					width,
					height,
					VDP_TRUE,
					&stock_vdp_surface[0]);
			stats = ctx->m_VdpBitmapSurfaceCreate(
					ctx->m_Device,
					VDP_RGBA_FORMAT_B8G8R8A8,
					width,
					height,
					VDP_TRUE,
					&stock_vdp_surface[1]);
			current_use_idx = 0;
		}
		auto next_surface = [&]() -> VdpBitmapSurface
		{
			if (current_use_idx == 0)
				return stock_vdp_surface[1];
			return stock_vdp_surface[0];
		};
		auto queue_next = [&]()
		{
			if (current_use_idx == 0)
			{
				current_use_idx = 1;
			}
			else
			{
				current_use_idx = 0;
			}
		};

		// Main loop
		while (!m_noticeThreadClose)
		{
			// Start the Dear ImGui frame
			ImGui_ImplSDLRenderer_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();

			UI::Rendering();

			SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
			SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
			SDL_RenderClear(renderer);
			ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

			SDL_Surface *new_surface = surface;

			if (new_surface && ctx->m_Device &&
					ctx->m_VdpBitmapSurfaceCreate && ctx->m_VdpBitmapSurfacePutBitsNative && ctx->m_VdpBitmapSurfaceDestroy)
			{
				{
					VdpBitmapSurface vdp_surface = 0;

					vdp_surface = next_surface();
					VdpStatus stats = VDP_STATUS_OK;
					if (vdp_surface != 0)
					{
						if (vdp_surface != ctx->m_cachedVdpSurface)
						{
							stats = ctx->m_VdpBitmapSurfacePutBitsNative(
									vdp_surface,
									&new_surface->pixels,
									(const uint32_t *)&new_surface->pitch,
									nullptr);
							if (stats == VDP_STATUS_OK)
							{
								VdpBitmapSurface old_vdp_surface = 0;

								if (pthread_mutex_trylock(&ctx->m_vdpsurface_lock) == 0)
								{
									old_vdp_surface = ctx->m_cachedVdpSurface;

									ctx->m_cachedVdpSurface = vdp_surface;

									pthread_mutex_unlock(&ctx->m_vdpsurface_lock);

									queue_next();
								}
								else
								{
								}
							}
						}
						else
						{
							ctx->m_cachedVdpSurface = 0;
						}
					}
				}
			}
			util::msleep(10);
		}

		{
			pthread_mutex_lock(&ctx->m_vdpsurface_lock);
			if (ctx->m_cachedVdpSurface != 0)
			{
				ctx->m_cachedVdpSurface = 0;
			}
			pthread_mutex_unlock(&ctx->m_vdpsurface_lock);
		}

		ctx->m_VdpBitmapSurfaceDestroy(stock_vdp_surface[0]);
		ctx->m_VdpBitmapSurfaceDestroy(stock_vdp_surface[1]);

		UI::m_ImguiInited = false;
		// Cleanup
		ImGui_ImplSDLRenderer_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		SDL_DestroyRenderer(renderer);
		SDL_FreeSurface(surface);
		SDL_DestroyWindow(window);
	}
	void *unixRenderThread(void *a)
	{

		RenderThread(a);

		return 0;
	}

	__forceinline void UI_init()
	{

		m_renderContext = {0};
		pthread_mutex_init(&m_renderContext.m_vdpsurface_lock, NULL);

		m_noticeThreadClose = false;
		UI::m_ImguiInited = false;
		m_guiinited = false;

		pthread_create(&m_thread, 0, unixRenderThread, &m_renderContext);

		UI::m_newcursor_pos_inited = false;
		m_guiinited = true;
	}
	__forceinline void UI_update(void *context)
	{
		// std::cout << "update\r\n";
		// return;
		// printf("context:%p\r\n",context);
		if (UI::m_ImguiInited)
		{
			struct MyGuiContext
			{
				int displayWidth;
				int displayHeight;
				void *m_VdpOutputSurfaceRenderBitmapSurface;
				void *m_VdpBitmapSurfaceDestroy;
				void *m_VdpBitmapSurfaceCreate;
				void *m_VdpBitmapSurfacePutBitsNative;
				VdpDevice m_Device;
				void *VdpOutputSurfaceRenderBlendState;
				VdpOutputSurface chosenSurface;
			};
			MyGuiContext *ctx = (MyGuiContext *)context;
			auto m_VdpOutputSurfaceRenderBitmapSurface = (VdpOutputSurfaceRenderBitmapSurface *)ctx->m_VdpOutputSurfaceRenderBitmapSurface;
			auto m_VdpBitmapSurfaceDestroy = (VdpBitmapSurfaceDestroy *)ctx->m_VdpBitmapSurfaceDestroy;
			auto m_VdpBitmapSurfaceCreate = (VdpBitmapSurfaceCreate *)ctx->m_VdpBitmapSurfaceCreate;
			auto m_VdpBitmapSurfacePutBitsNative = (VdpBitmapSurfacePutBitsNative *)ctx->m_VdpBitmapSurfacePutBitsNative;
			VdpDevice m_Device = ctx->m_Device;
			int displayWidth = ctx->displayWidth;
			int displayHeight = ctx->displayHeight;
			auto m_VdpOutputSurfaceRenderBlendState = (VdpOutputSurfaceRenderBlendState *)ctx->VdpOutputSurfaceRenderBlendState;

			// p1x(m_VdpOutputSurfaceRenderBitmapSurface);
			// p1x(m_VdpBitmapSurfaceDestroy);
			// p1x(m_VdpBitmapSurfaceCreate);
			// p1x(m_VdpBitmapSurfacePutBitsNative);
			// p1x(m_Device);
			// p1x(displayWidth);
			// p1x(displayHeight);
			// p1x(m_VdpOutputSurfaceRenderBlendState);
			// p1x(ctx->chosenSurface);

			{
				if (m_renderContext.m_VdpBitmapSurfaceCreate == 0)
					m_renderContext.m_VdpBitmapSurfaceCreate = m_VdpBitmapSurfaceCreate;
				if (m_renderContext.m_VdpBitmapSurfaceDestroy == 0)
					m_renderContext.m_VdpBitmapSurfaceDestroy = m_VdpBitmapSurfaceDestroy;
				if (m_renderContext.m_VdpOutputSurfaceRenderBitmapSurface == 0)
					m_renderContext.m_VdpOutputSurfaceRenderBitmapSurface = m_VdpOutputSurfaceRenderBitmapSurface;
				if (m_renderContext.m_VdpBitmapSurfacePutBitsNative == 0)
					m_renderContext.m_VdpBitmapSurfacePutBitsNative = m_VdpBitmapSurfacePutBitsNative;
				if (m_renderContext.m_Device == 0)
				{
					m_renderContext.m_Device = m_Device;
				}
			}

			while (pthread_mutex_trylock(&m_renderContext.m_vdpsurface_lock) != 0)
				;
			// pthread_mutex_lock(&m_renderContext.m_vdpsurface_lock);
			// m_renderContext.m_VdpBitmapSurfaceCreate = m_VdpBitmapSurfaceCreate;
			// m_renderContext.m_VdpBitmapSurfaceDestroy = m_VdpBitmapSurfaceDestroy;
			// m_renderContext.m_VdpOutputSurfaceRenderBitmapSurface = m_VdpOutputSurfaceRenderBitmapSurface;
			// m_renderContext.m_VdpBitmapSurfacePutBitsNative = m_VdpBitmapSurfacePutBitsNative;
			// m_renderContext.m_Device = m_Device;

			VdpBitmapSurface usingVdpSurface = m_renderContext.m_cachedVdpSurface;
			// pt("rendering...\r\n");

			if (usingVdpSurface)
			{
				// p1x(usingVdpSurface);

				VdpStatus stats = VDP_STATUS_OK;

				VdpRect rec;
				rec.x0 = 0;
				rec.y0 = 0;
				rec.x1 = displayWidth;
				rec.y1 = displayHeight;

				VdpOutputSurfaceRenderBlendState m_OverlayBlendState = {0};
				m_OverlayBlendState.struct_version = VDP_OUTPUT_SURFACE_RENDER_BLEND_STATE_VERSION;
				m_OverlayBlendState.blend_factor_destination_alpha = VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				m_OverlayBlendState.blend_factor_destination_color = VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				m_OverlayBlendState.blend_factor_source_alpha = VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_SRC_ALPHA;
				m_OverlayBlendState.blend_factor_source_color = VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_SRC_ALPHA;
				m_OverlayBlendState.blend_equation_alpha = VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_ADD;
				m_OverlayBlendState.blend_equation_color = VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_ADD;
				m_OverlayBlendState.blend_constant = {};

				// VdpRect rec;
				m_VdpOutputSurfaceRenderBitmapSurface(
						ctx->chosenSurface,
						&rec,
						usingVdpSurface,
						nullptr,
						nullptr,
						m_VdpOutputSurfaceRenderBlendState,
						0);
			}
			pthread_mutex_unlock(&m_renderContext.m_vdpsurface_lock);
			// m_renderContext.m_vdpsurface_lock = 0;
			// puts("ok");
		}
	}
	__forceinline void UI_destory()
	{
		if (m_guiinited)
		{
			m_noticeThreadClose = true;
			pthread_join(m_thread, 0);
			pthread_mutex_destroy(&m_renderContext.m_vdpsurface_lock);
		}
		UI::m_ImguiInited = false;
		m_guiinited = false;
		UI::m_newcursor_pos_inited = false;
	}
}

namespace UI_InputQueue
{
	int m_is_cancel = 0;
	pthread_t m_thread = 0;
	void *ThreadProcessInputQueue(void *)
	{
		while (true)
		{
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += 1;  // 等待一秒
			if(sem_timedwait(UI::m_frontend->sem(),&ts) == 0)
			{
				universal_mousedata mdata;
				while (UI::m_frontend->mouse_dequeue(mdata) == true)
				{
					if (!UI::m_showui && UI::m_ImguiInited)
					{
						if (mdata.type == UNIVERSAL_MOUSEMOVE)
						{
							SDL_Event event;
							event.type = SDL_MOUSEMOTION;
							event.motion.x = 0;										// 鼠标位置的x坐标
							event.motion.y = 0;										// 鼠标位置的y坐标
							event.motion.xrel = mdata.dx;					// 鼠标在x方向上移动的距离
							event.motion.yrel = mdata.dy;					// 鼠标在y方向上移动的距离
							event.motion.state = SDL_BUTTON_LEFT; // 鼠标状态，这里假设左键被按下

							SDL_PushEvent(&event);
						}
					}
				}
			}
			if (m_is_cancel == 1)
				break;
			//util::msleep(1);
		}
		return 0;
	}

	void LaunchThread()
	{
		// return;
		m_is_cancel = 0;
		pthread_create(&m_thread, 0, ThreadProcessInputQueue, 0);
	}
	void CancelThread()
	{
		// return;
		m_is_cancel = 1;
		pthread_cancel(m_thread);
		pthread_join(m_thread, 0);
	}

}

namespace UI
{
	bool is_egl = 0;

	void UI_init(SDL_Window *window, SDL_GLContext ctx)
	{
		if ((uint64_t)window == 0x036946395817681d && (uint64_t)ctx == 0x7fb4619066192d58)
		{
			UI_VDPAU::UI_init();
			is_egl = false;
		}
		else
		{
			UI_EGL::UI_init(window, ctx);
			is_egl = true;
		}

		UI_InputQueue::LaunchThread();
		puts("initok");
	}
	void UI_update(void *ctx)
	{
		puts("update");
		if (is_egl)
		{
			UI_EGL::UI_update();
		}
		else
		{
			UI_VDPAU::UI_update(ctx);
		}
	}
	void UI_destory()
	{

		puts("destory");
		UI_InputQueue::CancelThread();

		if (is_egl)
		{
			UI_EGL::UI_destory();
		}
		else
		{
			UI_VDPAU::UI_destory();
		}
		is_egl = false;
	}
}
namespace UI
{
	void InitFrontEND(universal_shm *shm)
	{
		UI::m_frontend = std::make_shared<frontend>(shm);
	}
}

extern "C"
{
	void MYGUI_loop()
	{
		return UI::UI_loop();
	}
	bool MYGUI_event(SDL_Event *Event)
	{
		return UI::UI_event(Event);
	}
	void MYGUI_init(SDL_Window *window, SDL_GLContext ctx)
	{
		return UI::UI_init(window, ctx);
	}
	void MYGUI_update(void *ctx)
	{
		return UI::UI_update(ctx);
	}
	void MYGUI_destory()
	{
		return UI::UI_destory();
	}
}