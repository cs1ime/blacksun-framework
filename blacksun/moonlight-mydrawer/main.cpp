#include <forceinline.h>
#include <stdio.h>
#include <SDL.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "gui.h"
#include <sys/mman.h>
#include <protocol-universal-shm.h>

// #if SDL_COMPILEDVERSION != SDL_VERSIONNUM(2, 26, 2)
// #error This framework must use SDL 2.26.2
// #endif


uint64_t g_funcs[100];
__attribute__((visibility("default")))
extern "C" uint64_t* GetFuncs() {
    universal_shm *shm= universal_shm_create_and_attach(UNIVERSAL_SHM_KEY);
    shm->initialize();

    UI::InitFrontEND(shm);

    g_funcs[0] = (uint64_t) MYGUI_init;
    g_funcs[1] = (uint64_t) MYGUI_update;
    g_funcs[2] = (uint64_t) MYGUI_destory;
    g_funcs[3] = (uint64_t) MYGUI_event;
    g_funcs[4] = (uint64_t) MYGUI_loop;

    return g_funcs;
}

