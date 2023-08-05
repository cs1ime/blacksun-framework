#pragma once
#ifndef _OFFSETS_
#define _OFFSETS_
#include "windows.h"

namespace offsets {

	extern DWORD64 entity_list;
	extern DWORD64 local_player_id;
	extern DWORD64 NameList;
	extern DWORD64 NetVarPtr;
	extern DWORD64 ViewRender;
	extern DWORD64 Matrix;

	extern DWORD64 health;
	extern DWORD64 bones;
	extern DWORD64 iSignifierName;
	extern DWORD64 base_pos;
	extern DWORD64 script_id;
	extern DWORD64 latestPrimaryWeapons;
	extern DWORD64 bleedoutState;
	extern DWORD64 vecAbsVelocity;
	extern DWORD64 ObserverTarget;

#define GLOW_T1              0x262 //16256 = enabled, 0 = disabled 
#define GLOW_T2              0x2dc //1193322764 = enabled, 0 = disabled 
#define GLOW_ENABLE          0x3c8 //7 = enabled, 2 = disabled
#define GLOW_THROUGH_WALLS   0x3d0 //2 = enabled, 5 = disabled

#define GLOW_FADE 0x388
#define GLOW_DISTANCE 0x3B4
#define GLOW_CONTEXT 0x3C8
#define GLOW_LIFE_TIME 0x3A4
#define GLOW_VISIBLE_TYPE 0x3D0
#define GLOW_TYPE 0x2C4
#define GLOW_COLOR 0x1D0
#define GLOW_COLOR2 0x1D4 // GLOW_COLOR + 0x4
#define GLOW_COLOR3 0x1D8 // GLOW_COLOR + 0x8

	extern DWORD64 CameraPos;
	extern DWORD64 TeamNumber;
	extern DWORD64 id;

	extern DWORD64 bullet_velocity;
	extern DWORD64 velocity;
	extern DWORD64 aim_punch;
	extern DWORD64 DynamicAngle;
	extern DWORD64 view_angles;
	extern DWORD64 VisibleTime;
	extern DWORD64 local_player;

	bool InstallOffsets();
	bool InstallOffsetsDisableSmap();
}

#endif // !_OFFSETS_