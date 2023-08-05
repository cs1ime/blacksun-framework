#pragma once
#ifndef _OFFSETS_
#define _OFFSETS_
#include "windows.h"

namespace offsets {
	const DWORD64 id = 0x8;

	const DWORD64 health = 0x43c;
	const DWORD64 health_max = 0x578;
	const DWORD64 iSignifierName = 0x580;
	const DWORD64 TeamNumber = 0x44c;
	const DWORD64 base_pos = 0x14c;
	const DWORD64 latestPrimaryWeapons = 0x1a14;
	const DWORD64 bleedoutState = 0x2750;
	const DWORD64 velocity = 0x488;
	const DWORD64 ObserverTarget = 0x3500;
	const DWORD64 ViewRender = 0x743bca0;
	const DWORD64 Matrix = 0x11a350;
	const DWORD64 local_player = 0x22048c8;
	const DWORD64 entity_list = 0x1e54dc8;
	const DWORD64 view_angles = 0x25a0;
	const DWORD64 DynamicAngle = 0x2590;
	const DWORD64 CameraPos = 0x1f50;
	const DWORD64 aim_punch = 0x24b8;

	const DWORD64 bones = 0x0e98 + 0x48;
	const DWORD64 bullet_velocity = 0x1F3C;
	const DWORD64 VisibleTime = 0x1A70;
};
#endif // !_OFFSETS_