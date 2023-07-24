#ifndef MYGUI_H
#define MYGUI_H

#include <SDL.h>

#define __forceinline __attribute__((always_inline)) inline

namespace mygui{
typedef void (*FN_MYGUI_init)(SDL_Window* window,SDL_GLContext ctx);
typedef void (*FN_MYGUI_update)(void *ctx);
typedef void (*FN_MYGUI_destory)();
typedef bool (*FN_MYGUI_event)(SDL_Event *);
typedef void (*FN_MYGUI_loop)();

extern FN_MYGUI_init g_PFN_MYGUI_init;
extern FN_MYGUI_update g_PFN_MYGUI_update;
extern FN_MYGUI_destory g_PFN_MYGUI_destory;
extern FN_MYGUI_event g_PFN_MYGUI_event;
extern FN_MYGUI_loop g_PFN_MYGUI_loop;

    void init();
    __forceinline void InvokeInit(SDL_Window* window,SDL_GLContext ctx){
        if(g_PFN_MYGUI_init!=nullptr){
            g_PFN_MYGUI_init(window,ctx);
        }
    }
    __forceinline void InvokeDestory(){
        if(g_PFN_MYGUI_destory!=nullptr){
            g_PFN_MYGUI_destory();
        }
    }
    __forceinline void InvokeUpdate(void *ctx=0){
        if(g_PFN_MYGUI_update!=nullptr){
            g_PFN_MYGUI_update(ctx);
        }
    }
    __forceinline bool InvokeEvent(SDL_Event* event){
        if(g_PFN_MYGUI_event!=nullptr){
            return g_PFN_MYGUI_event(event);
        }
        return true;
    }
    __forceinline void InvokeLoop(){
        if(g_PFN_MYGUI_loop!=nullptr){
            g_PFN_MYGUI_loop();
        }
    }

}

#endif // MYGUI_H
