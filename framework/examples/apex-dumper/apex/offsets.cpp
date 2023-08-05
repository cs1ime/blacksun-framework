#include "offsets.h"
#include "NetVars.h"
#define xs
#include "Vars.h"
#include "CompatibleLayer.h"

#define _ptb


namespace offsets {
	DWORD64 ImgSz = 0x1F5C9000;

	DWORD64 entity_list = 0x1e54dc8;
	DWORD64 local_player_id = 0x12E42BC;
	DWORD64 local_player = 0;
	DWORD64 NameList = 0;
	DWORD64 NetVarPtr = 0;
	DWORD64 ViewRender = 0;
	DWORD64 Matrix = 0;

	DWORD64 bones = 0xED0;

	DWORD64 health = 0x43C;
	DWORD64 iSignifierName = 0x580;
	DWORD64 base_pos = 0x14C;
	DWORD64 latestPrimaryWeapons = 0x19EC;
	DWORD64 bleedoutState = 0x26D0;
	DWORD64 velocity = 0x488;
	DWORD64 ObserverTarget = 0x3470;

	DWORD64 CameraPos = 0;
	DWORD64 TeamNumber = 0;
	DWORD64 id = 0x8;

	DWORD64 bullet_velocity = 0x1ED0; //F3 0F 10 87 ?? ?? ?? ?? 45 0F 57 C0
	
	DWORD64 aim_punch = 0x2440; // F2 41 0F 10 9F ?? ?? ?? ?? 0F 28 E0
	DWORD64 DynamicAngle = 0x2518;
	DWORD64 view_angles = 0x2528;
	DWORD64 VisibleTime = 0x1A48; //8B ?? ?? ?? 00 00 89 ?? 48 8D 15 特征字符串lastVisibleTime


	DWORD64 vecAbsVelocity = 0;
	DWORD64 script_id = 0;
}

ULONG64 solve(ULONG64 pos, LONG offset, LONG Next) {
	LONG v = 0;
	if (MmiGetPhysicalAddress((PVOID)(pos + offset))) {
		MmiReadVirtualAddressSafe(pos + offset, &v, 4);
		return v + pos + Next;
	}
	return 0;
}

namespace offsets {
//#pragma optimize( "", off )
//#pragma clang optimize off
	bool InstallOffsets() {
		_ptb("install!");
		_ptb("pGameImage:%p", Vars::pGameImage);
		bool g_enableprint = 1;
		ULONG64 pos = 0;
		pos = ScanMemory(Vars::pGameImage, ImgSz, "E8 EB DF F1 FF");
		pos = ScanMemory(Vars::pGameImage, ImgSz, xs("4C 8B 1D ?? ?? ?? ?? 4D 85 DB 74 19"));
		_ptb("pos:%p", pos);
		bool result = false;
		if (pos) {
			result = true;
			ULONG64 addr = solve(pos, 3, 7);
			if (addr) {
				NetVarPtr = addr - Vars::pGameImage;
				ULONG64 cls = 0;
				if (MmiGetPhysicalAddress((PVOID)(NetVarPtr + Vars::pGameImage))) {
					//memcpy(&cls, (PVOID)(offset::offset_NetVarPtr + Global::pGameImage), 8);
					MmiReadVirtualAddressSafe(NetVarPtr + Vars::pGameImage, &cls, 8);
					CNetVars::Initialize(cls);
					CNetVars::PrintAll(cls);

					health = CNetVars::GetOffset(xs("DT_Player"), xs("m_iHealth"));
					int health_max = CNetVars::GetOffset(xs("DT_Player"), xs("m_iMaxHealth"));
					iSignifierName = CNetVars::GetOffset(xs("DT_BaseEntity"), xs("m_iSignifierName"));
					TeamNumber = CNetVars::GetOffset(xs("DT_BaseEntity"), xs("m_iTeamNum"));
					vecAbsVelocity = CNetVars::GetOffset(xs("DT_Player"), xs("m_vecAbsVelocity"));
					base_pos = vecAbsVelocity + 0xC;
					script_id = CNetVars::GetOffset(xs("DT_PropSurvival"), xs("m_customScriptInt"));
					latestPrimaryWeapons = CNetVars::GetOffset(xs("DT_Player"), xs("m_latestPrimaryWeapons"));
					bleedoutState = CNetVars::GetOffset(xs("DT_Player"), xs("m_bleedoutState"));
					velocity = CNetVars::GetOffset(xs("DT_Projectile"), xs("m_vecVelocity"));
					ObserverTarget = CNetVars::GetOffset(xs("DT_Player"), xs("m_hObserverTarget"));

					printf("const DWORD64 health = %p\n", health);
					printf("const DWORD64 health_max = %p\n", health_max);
					printf("const DWORD64 iSignifierName = %p\n", iSignifierName);
					printf("const DWORD64 TeamNumber = %p\n", TeamNumber);
					printf("const DWORD64 base_pos = %p\n", base_pos);
					printf("const DWORD64 latestPrimaryWeapons = %p\n", latestPrimaryWeapons);
					printf("const DWORD64 bleedoutState = %p\n", bleedoutState);
					printf("const DWORD64 velocity = %p\n", velocity);
					printf("const DWORD64 ObserverTarget = %p\n", ObserverTarget);


				}
			}
		}
		//48 89 0D ?? ?? ?? ?? FF 10 48 8B 05
		pos = ScanMemory(Vars::pGameImage, ImgSz, xs("48 8B 05 ?? ?? ?? ?? 0F B7 90 ?? ?? ?? 00 66 41 3B D0"));
		if (pos) {
			ViewRender = solve(pos, 3, 7) - Vars::pGameImage;
			if (g_enableprint) {
				printf(xs("const DWORD64 ViewRender = %p\n"), ViewRender);
			}
		}
		else {
			exit(1);
		}

		//4C 8D 85 ?? ?? ?? ?? ?? 8B ?? ?? ?? ?? ?? 48 8D 15
		pos = ScanMemory(Vars::pGameImage, ImgSz, xs("4C 8D 85 ?? ?? ?? ?? ?? 8B ?? ?? ?? ?? ?? 48 8D 15"));
		if (pos) {
			ULONG v = 0;
			if (MmiGetPhysicalAddress((PVOID)(pos + 10))) {
				//memcpy(&v, (PVOID)(FindPos.GetPos() + 10), 4);
				MmiReadVirtualAddressSafe(pos + 10, &v, 4);
				Matrix = v;
				if (g_enableprint) {
					printf(xs("const DWORD64 Matrix = %p\n"), Matrix);
				}
			}
		}
		else {
			exit(1);
		}
		//38 48 8B 05 ?? ?? ?? ?? FF 50 20
		pos = ScanMemory(Vars::pGameImage, ImgSz, xs("38 48 8B 05 ?? ?? ?? ?? FF 50 20"));
		if (pos) {
			local_player = solve(pos, 4, 8) - Vars::pGameImage;
			local_player += 8;
			if (g_enableprint) {
				printf(xs("const DWORD64 local_player = %p\n"), local_player);
			}
		}
		else {
			exit(1);
		}
		//488D0D????????41894310
		//4883EC28BAA00F0000488D0D????????FF15????????4533C9488D05????????4C890D????????488D0D????????6690 
		//0F B7 C8 48 8D 15 ?? ?? ?? ?? 48 C1 E1 05 C1 E8 10 39 44 11 08
		pos = ScanMemory(Vars::pGameImage, ImgSz, xs("0F B7 C8 48 8D 15 ?? ?? ?? ?? 48 C1 E1 05 C1 E8 10 39 44 11 08"));
		if (pos) {
			entity_list = solve(pos, 6, 10) - Vars::pGameImage;
			if (g_enableprint) {
				printf(xs("const DWORD64 entity_list = %p\n"), entity_list);
			}
		}
		else {
			exit(1);
		}
		//ViewAngle:88 83 ?? ?? ?? ?? 4C 8D 87 ?? ?? ?? ?? 8B 83 
		//DymAngle:F3 0F 5D CB 49 8D 8A ?? ?? ?? ?? 
		pos = ScanMemory(Vars::pGameImage, ImgSz, xs("88 83 ?? ?? ?? ?? 4C 8D 87 ?? ?? ?? ?? 8B 83"));
		if (pos) {
			ULONG v = 0;
			if (MmiGetPhysicalAddress((PVOID)(pos + 9))) {
				//memcpy(&v, (PVOID)(FindPos.GetPos() + 9), 4);
				MmiReadVirtualAddressSafe(pos + 9, &v, 4);
				view_angles = v;
				DynamicAngle = view_angles - 0x10;
				if (g_enableprint) {
					printf(xs("const DWORD64 view_angles = %p\n"), view_angles);
					printf(xs("const DWORD64 DynamicAngle = %p\n"), DynamicAngle);
				}
			}
		}
		else {
			exit(1);
		}
		//F3 0F 10 0A 48 8B F9 0F 2E 89 ?? ?? ?? ??
		pos = ScanMemory(Vars::pGameImage, ImgSz, xs("F3 0F 10 0A 48 8B F9 0F 2E 89 ?? ?? ?? ??"));
		if (pos) {
			ULONG v = 0;
			if (MmiGetPhysicalAddress((PVOID)(pos + 10))) {
				//memcpy(&v, (PVOID)(FindPos.GetPos() + 10), 4);
				MmiReadVirtualAddressSafe(pos + 10, &v, 4);
				CameraPos = v;
				if (g_enableprint) {
					printf(xs("const DWORD64 CameraPos = %p\n"), CameraPos);
				}
			}
		}
		else {
			exit(1);
		}

		//F2 41 0F 10 9F ?? ?? ?? ?? 0F 28 E0
		pos = ScanMemory(Vars::pGameImage, ImgSz, xs("F2 41 0F 10 9F ?? ?? ?? ?? 0F 28 E0"));
		if (pos) {
			ULONG v = 0;
			if (MmiGetPhysicalAddress((PVOID)(pos + 5))) {
				//memcpy(&v, (PVOID)(FindPos.GetPos() + 10), 4);
				MmiReadVirtualAddressSafe(pos + 5, &v, 4);
				aim_punch = v;
				if (g_enableprint) {
					printf(xs("const DWORD64 aim_punch = %p\n"), aim_punch);
				}
			}
		}
		else {
			exit(1);
		}




		return result;
	}
	bool InstallOffsetsDisableSmap() {
		//_CR4 cr4;
		//cr4.all = AsmReadCr4();
		//bool smap = cr4.SMAP == 1;
		//if (smap == true) {
		//	cr4.SMAP = 0;
		//	AsmWriteCr4(cr4.all);
		//}

		return InstallOffsets();

		//if (smap == true) {
		//	cr4.SMAP = 1;
		//	AsmWriteCr4(cr4.all);
		//}
	}
//#pragma clang optimize on
//#pragma optimize( "", on )
}