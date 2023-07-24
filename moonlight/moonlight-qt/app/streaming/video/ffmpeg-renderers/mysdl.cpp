#include <iostream>
#include <imgui.h>
#include <imgui_impl_sdl2.h>>
#include <imgui_impl_sdlrenderer.h>>
#include <SDL.h>
#include <SDL_surface.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h>

#define InterlockedCompareExchange16(a1,a2,a3)(__sync_val_compare_and_swap(a1,a3,a2))

namespace mysdl {
    SDL_Renderer* m_mainRenderer=0;
    SDL_Window* m_mainWindow=0;

    SDL_Renderer *m_recverCopyRender=0;
    SDL_Surface m_recverCopySurface={0};



    int msleep(long msec)
    {
        struct timespec ts;
        int res;

        if (msec < 0)
        {
            errno = EINVAL;
            return -1;
        }

        ts.tv_sec = msec / 1000;
        ts.tv_nsec = (msec % 1000) * 1000000;

        do {
            res = nanosleep(&ts, &ts);
        } while (res && errno == EINTR);

        return res;
    }
    SDL_Surface * CopySurface(SDL_Renderer *renderer,SDL_Surface * wsurface){

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, wsurface);
        SDL_Texture* target = SDL_GetRenderTarget(renderer);
        SDL_SetRenderTarget(renderer, texture);
        int width, height;
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
        SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
        SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);

        SDL_SetRenderTarget(renderer, target);
        SDL_DestroyTexture(texture);

        return surface;
    }

    short cache_lock=0;
    SDL_Surface *cache_surface=0;

    SDL_Surface *recv_cache_surface=0;
    SDL_Surface *GetSurface(){
        //if(InterlockedCompareExchange16(&cache_lock,1,0)==0){
        //    if(cache_surface!=0){
        //        if(recv_cache_surface!=0){
        //            SDL_FreeSurface(recv_cache_surface);
        //            recv_cache_surface=0;
        //        }


        //        recv_cache_surface=CopySurface(m_recverCopyRender,cache_surface);
        //    }
        //    cache_lock=0;
        //}
        return cache_surface;
    }
    void *unixthread(void *){

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.00f);
        while(1){
            SDL_Event event;
             while (SDL_PollEvent(&event))
             {
                 ImGui_ImplSDL2_ProcessEvent(&event);
                 if (event.type == SDL_QUIT)
                     return 0;
                 if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_mainWindow))
                     return 0;
             }
            ImGui_ImplSDLRenderer_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            ImGuiIO& io = ImGui::GetIO(); (void)io;

            ImDrawList *list=ImGui::GetForegroundDrawList();
            list->AddLine(ImVec2(0,0),ImVec2(100,100),ImColor(255,0,0));

            ImGui::Render();
            SDL_RenderSetScale(m_mainRenderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
            SDL_SetRenderDrawColor(m_mainRenderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
            SDL_RenderClear(m_mainRenderer);
            ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
            SDL_RenderPresent(m_mainRenderer);

            {
                SDL_Surface* wsurface = SDL_GetWindowSurface(m_mainWindow);
                SDL_Surface* surface=CopySurface(m_mainRenderer,wsurface);
                if(surface!=0){
                    while(InterlockedCompareExchange16(&cache_lock,1,0)==1)
                        msleep(1);

                    //if(cache_surface!=0)
                    //    SDL_FreeSurface(cache_surface);
                    //cache_surface=0;
                    cache_surface=surface;

                    cache_lock=0;
                }

            }


            msleep(20);
        }


        return 0;
    }

    void init(){
        //SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        m_mainWindow = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, window_flags);
        m_mainRenderer = SDL_CreateRenderer(m_mainWindow, -1, SDL_RENDERER_ACCELERATED);

        SDL_Window *window2 = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, window_flags);
        //m_recverCopyRender=SDL_CreateSoftwareRenderer(&m_recverCopySurface);
        m_recverCopyRender = SDL_CreateRenderer(window2, -1, SDL_RENDERER_ACCELERATED);


        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForSDLRenderer(m_mainWindow, m_mainRenderer);
        ImGui_ImplSDLRenderer_Init(m_mainRenderer);

        pthread_t thread_id;
        pthread_create(&thread_id,0,unixthread,0);
    }
    void tryinit(){

        static bool is_inited=false;
        if(is_inited==false){
            init();
            is_inited=true;
        }


    }


}



