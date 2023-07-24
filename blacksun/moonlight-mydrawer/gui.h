#pragma once
#ifndef _MYGUI_H_
#define _MYGUI_H_

#include <SDL.h>
#include <protocol-universal-shm.h>

namespace UI {
	void mousemove(int x, int y);
    void InitFrontEND(universal_shm *shm);
}

extern "C" {
	void MYGUI_loop();
	bool MYGUI_event(SDL_Event* Event);
	void MYGUI_init(SDL_Window* window, SDL_GLContext ctx);
	void MYGUI_update(void *ctx);
	void MYGUI_destory();
}



#endif // !_MYGUI_H_
