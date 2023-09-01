#pragma once
#ifndef _OFFSETS_
#define _OFFSETS_
#include "windows.h"
// 2023-09-01 16:29:46
// GameVersion=v3.0.42.35
namespace offsets {
	const DWORD64 id = 0x8;

	const DWORD64 health = 0x470;//m_iHealth
	const DWORD64 health_max = 0x5b0;//m_iMaxHealth

	const DWORD64 shield = 0x1a0; //m_shieldHealth
    const DWORD64 shield_max = 0x1a4; //m_shieldHealthMax

	const DWORD64 iSignifierName = 0x5b8;//m_iSignifierName
	const DWORD64 TeamNumber = 0x480;//m_iTeamNum
	const DWORD64 base_pos = 0x17c;
	const DWORD64 latestPrimaryWeapons = 0x1a44;//m_latestPrimaryWeapons
	const DWORD64 bleedoutState = 0x2790;//m_bleedoutState
	const DWORD64 velocity = 0x4bc;//m_vecVelocity
	const DWORD64 ObserverTarget = 0x3540;//m_hObserverTarget

	const DWORD64 ViewRender = 0x7472a28;//ViewRender
	const DWORD64 Matrix = 0x11a350;//ViewMatrix
	const DWORD64 local_player = 0x2225640;//LocalPlayer
	const DWORD64 entity_list = 0x1e754c8;//cl_entitylist

	const DWORD64 view_angles = 0x25e4 - 0x14;//m_ammoPoolCapacity - 0x14
	const DWORD64 DynamicAngle = view_angles - 0x10;
	const DWORD64 CameraPos = 0x1f80;//CPlayer!camera_origin
	const DWORD64 aim_punch = 0x24e8;//m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle

	const DWORD64 bones = 0xec8 + 0x48;//m_nForceBone + 0x48
	const DWORD64 bullet_velocity = 0x1f6c;//CWeaponX!m_flProjectileSpeed
	const DWORD64 VisibleTime = 0x1aa0;//CPlayer!lastVisibleTime
};
#endif // !_OFFSETS_
