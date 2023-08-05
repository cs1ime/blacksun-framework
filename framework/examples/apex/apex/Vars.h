#pragma once

#ifndef _VARS_H_
#define _VARS_H_

#include "ParserStruct.h"
#include "windows.h"

#define MAX_PLAYER (100)
#define BONE_BUFFER_MAXSIZE (480)

namespace Vars {
	extern ULONG64 pGameImage;
	extern BOOLEAN InstalledOffsets;

	extern Player PlayerList[MAX_PLAYER];
	extern int PlayerCount;
	extern UCHAR BoneBuffer[BONE_BUFFER_MAXSIZE * MAX_PLAYER];
	extern int BoneBufferIndex;

	extern ULONG64 LocalPlayer;
	extern USHORT LocalPlayerId;
	extern ULONG64 LocalWeap;
	extern ULONG LocalWeapId;
	extern vec3_t LocalWeapSpeed;
	extern ULONG LocalTeamId;
	extern vec3_t LocalPlayerPos;
	extern vec3_t LocalPlayerCameraPos;
	extern vec3_t LocalViewAngle;
	extern vec3_t LocalDynamicAngle;
	extern vec3_t LocalAimPunch;

	extern Player* AimPlayer;
	extern int m_uObseredCount;
}
namespace Flag {
	extern bool Aimbot;
	extern bool user_ESP;
	extern bool user_ESP_item;
	extern bool user_ESP_Glow;
	extern bool user_aimbot_on;
	extern int user_aimbot_state;
	extern bool user_aimbot_notdown;
	extern bool user_aimbot_continue;
	extern float user_Aimbot_Range;
	extern float user_Aimbot_smooth;
	extern bool smart_aim;

	extern float user_ESP_distance;
}


#endif // !_VARS_H_

