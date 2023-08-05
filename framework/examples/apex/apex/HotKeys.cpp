#include "HotKeys.h"
#include "Vars.h"
#include "xorstr.hpp"
#include <protocol-backend.h>
#include "CompatibleLayer.h"

#include "windows.h"

extern backend *m_backend;

namespace SdlHotkey{
    bool IsBtnUp(int k){
        return m_backend->hkey()->IsBtnUp(k);
    }
    bool IsBtnDown(int k){
        return m_backend->hkey()->IsBtnDown(k);
    }
    bool IsKeyUp(int k){
        return m_backend->hkey()->IsKeyUp(k);
    }
    bool IsKeyDown(int k){
        return m_backend->hkey()->IsKeyDown(k);
    }
}
#pragma optimize( "", off )

__forceinline bool IsAimKeyUp() {
    return SdlHotkey::IsBtnUp(SDL_BUTTON_X1);
}
__forceinline bool IsAimKeyDown() {
    return SdlHotkey::IsBtnDown(SDL_BUTTON_X1);
}

void DispatchHotkey() {
	
	static bool aimbot_once = false;

	if (IsAimKeyDown())
	{
		if (Flag::user_aimbot_continue) {
			Flag::Aimbot = true;
		}
		else {
			if (aimbot_once == false) {
				Flag::Aimbot = true;
				aimbot_once = true;
			}
		}
	}
	if (IsAimKeyUp()) {
		aimbot_once = false;
		Flag::Aimbot = false;
	}
}

#pragma optimize( "", on )
