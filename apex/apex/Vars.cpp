#include "Vars.h"

#include "windows.h"

namespace Vars {
	ULONG64 pGameImage = 0;
	BOOLEAN InstalledOffsets = FALSE;

	Player PlayerList[MAX_PLAYER];
	int PlayerCount = 0;
	UCHAR BoneBuffer[BONE_BUFFER_MAXSIZE * MAX_PLAYER];
	int BoneBufferIndex = 0;

	ULONG64 LocalPlayer = 0;
	USHORT LocalPlayerId = 0;
	ULONG64 LocalWeap = 0;
	ULONG LocalWeapId = 0;
	vec3_t LocalWeapSpeed = { 0,0,0 };
	ULONG LocalTeamId = 0;
	vec3_t LocalPlayerPos = { 0,0,0 };
	vec3_t LocalPlayerCameraPos = { 0,0,0 };
	vec3_t LocalViewAngle = { 0,0,0 };
	vec3_t LocalDynamicAngle = { 0,0,0 };
	vec3_t LocalAimPunch = { 0,0,0 };

	Player* AimPlayer = 0;
	int m_uObseredCount = 0;
}

namespace Flag {
	bool Aimbot = false;
	int aimbot_key = 0;//0
	bool user_aimbot_on = true;
	int user_aimbot_state = 1;
	bool smart_aim = true;
	bool user_aimbot_notdown = false;
	bool user_aimbot_continue = false;
	float user_Aimbot_Range = 30.0f;
	float user_Aimbot_smooth = 2.f;
  float user_ESP_distance = 1200.f;
	int user_resolver_value = 2;

}
