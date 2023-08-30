#include "windows.h"
#include "offsets.h"
#include "vector.h"
#include "Vars.h"
#include "CompatibleLayer.h"
#include "xorstr.hpp"
#include "vector.h"
#include <protocol-backend.h>
#include "AimFramework.h"
#include "HotKeys.h"
#include <print-conrtol.h>

class ImColor
{
public:
	uint8_t RGBA[4];

	ImColor(uint8_t R, uint8_t G, uint8_t B, uint8_t A = 255)
	{
		RGBA[0] = R;
		RGBA[1] = G;
		RGBA[2] = B;
		RGBA[3] = A;
	}

	ImColor(uint32_t val)
	{
		*(uint32_t *)&RGBA = val;
	}

	auto Get() const
	{
		return *(uint32_t *)&RGBA;
	}
};

// #define printf
#define _ptb printf

extern "C"
{
	unsigned int _fltused = 1;
}
extern "C" float atanf(float x);
extern "C" float sqrtf(float x);
extern "C" DWORD64 _handle_nanf(DWORD64)
{
	return 0;
}

#define PI (3.1415f)

UCHAR GlobalObjectCache[0x20 * 0x10000];
float GlobalVisibleTimeCache[0x10000];
bool GlobalVisibleCache[0x10000];

AimFramework m_aim;

void InitValues()
{
	memset(GlobalObjectCache, 0, sizeof(GlobalObjectCache));
	memset(GlobalVisibleTimeCache, 0, sizeof(GlobalVisibleTimeCache));
	memset(GlobalVisibleCache, 0, sizeof(GlobalVisibleCache));
}

vec3_t get_bone(Player *p, int index)
{
	if (p->OffBoneBuffer)
	{
		vec3_t boneVec;
		boneVec.x = p->pos.x + *(float *)(Vars::BoneBuffer + p->OffBoneBuffer - 1 + index * 0x30 + 0xC);
		boneVec.y = p->pos.y + *(float *)(Vars::BoneBuffer + p->OffBoneBuffer - 1 + index * 0x30 + 0x1C);
		boneVec.z = p->pos.z + *(float *)(Vars::BoneBuffer + p->OffBoneBuffer - 1 + index * 0x30 + 0x2C);
		return boneVec;
	}
	return {0.f, 0.f, 0.f};
}
vec2_t calc_viewoffset(vec3_t target)
{
	float x = target.x - Vars::LocalPlayerCameraPos.x;
	float y = target.y - Vars::LocalPlayerCameraPos.y;
	float z = target.z - Vars::LocalPlayerCameraPos.z;
	float mousex = Vars::LocalViewAngle.x;
	float mousey = Vars::LocalViewAngle.y;

	if (mousex < 0.f && mousex >= -180.f)
	{
		mousex = 180.f + (180.f + mousex); // 修正偏转角,不同游戏不一样
	}
	mousey *= -1.f;

	float anglex, angley;
	anglex = 0;
	if (x > 0 && y == 0)
		anglex = 0; // 第一象限
	if (x > 0 && y > 0)
		anglex = abs(atanf(y / x)) / PI * 180.f;
	if (x == 0 && y > 0)
		anglex = 90.f; // 第二象限
	if (x < 0 && y > 0)
		anglex = 90.f + abs(atanf(x / y)) / PI * 180.f;
	if (x < 0 && y == 0)
		anglex = 180.f; // 第三象限
	if (x < 0 && y < 0)
		anglex = 180.f + abs(atanf(y / x)) / PI * 180.f;
	if (x == 0 && y < 0)
		anglex = 270.f; // 第四象限
	if (x > 0 && y < 0)
		anglex = 270.f + abs(atanf(x / y)) / PI * 180.f;

	float aerfa = mousex - anglex;
	if (aerfa > 180.f)
	{
		aerfa = (360.f - aerfa) * -1;
	}
	if (aerfa < -180.f)
	{
		aerfa = (360.f + aerfa);
	}

	angley = atanf(z / sqrt(x * x + y * y)) / PI * 180.f;
	float beta = mousey - angley;
	return {aerfa, beta};
}
vec3_t calc_aimbot_pos(vec3_t target, vec3_t velocity)
{
	if (Vars::LocalWeapSpeed.x < 100.0f)
		return target;
	float fltime = target.dis(Vars::LocalPlayerCameraPos) / Vars::LocalWeapSpeed.x;
	float gravity = fltime * fltime * 350.0f * Vars::LocalWeapSpeed.z;
	fltime *= 1.1f;
	vec3_t result;
	result.x = target.x + velocity.x * fltime;
	result.y = target.y + velocity.y * fltime;
	result.z = target.z + velocity.z * fltime + gravity;
	return result;
}

extern backend *m_backend;
float m_mat[16] = {0.f};
float m_width = 0.f;
float m_height = 0.f;
bool w2s(vec3_t &from, vec3_t &to)
{
	float targetWidth = m_width;
	float targetHeight = m_height;
	float *m_vMatrix = m_mat;
	float w = m_vMatrix[12] * from.x + m_vMatrix[13] * from.y + m_vMatrix[14] * from.z + m_vMatrix[15];

	if (w < 0.01f)
		return false;

	to.x = m_vMatrix[0] * from.x + m_vMatrix[1] * from.y + m_vMatrix[2] * from.z + m_vMatrix[3];
	to.y = m_vMatrix[4] * from.x + m_vMatrix[5] * from.y + m_vMatrix[6] * from.z + m_vMatrix[7];

	float invw = 1.0f / w;
	to.x *= invw;
	to.y *= invw;

	float x = targetWidth / 2;
	float y = targetHeight / 2;

	x += 0.5 * to.x * targetWidth + 0.5;
	y -= 0.5 * to.y * targetHeight + 0.5;

	to.x = x;
	to.y = y;
	to.z = 0;
	return true;
}

// 绘制玩家信息，使用渲染发送器来绘制玩家角色的可视化表示
void DrawPlayers(rendsender *ctx)
{
	for (int i = 0; i < Vars::PlayerCount; i++)
	{
		Player *p = &Vars::PlayerList[i];

		// 获取玩家角色的位置
		vec3_t ppos = {}, pposh = {};
		ppos = p->pos;
		pposh = ppos;
		pposh.z += 65.f;

		vec3_t pos = {}, posh = {};
		// 将世界坐标转换为屏幕坐标
		if (w2s(ppos, pos) && w2s(pposh, posh))
		{
			// 计算角色边框的高度和宽度
			float h = pos.y - posh.y;
			float w = h / 2;

			// 根据角色可见性设置颜色
			uint32_t _Color = p->visible ? ImColor(255, 0, 0).Get() : ImColor(0, 0, 255).Get();
			if (p->life_state != 0)
			{
				_Color = ImColor(255, 255, 255).Get();
			}
			// 计算角色边框的起始位置
			float startx = posh.x - w / 2;
			float starty = posh.y;
			// 向渲染发送器添加绘制角色边框的命令
			ctx->AddCornBox(startx, starty, w, h, _Color);
			// ctx->AddBox(startx, starty, w, h, col);

			// 绘制血条和盾
			if (p->health_max > 0 && p->life_state == 0)
			{
				// 计算血条的比例
				_Color = ImColor(62, 189, 0).Get();
				float scale = (float)p->health / (float)p->health_max;
				;
				float shield_scale = 0;
				// _ptb("shield:%d | ", p->shield);
				// _ptb("shield_max:%d\r\n", p->shield_max);
				if (p->shield > 0)
				{
					shield_scale = (float)p->shield / (float)p->shield_max;
					if (p->shield_max == 50)
					{
						_Color = ImColor(255, 255, 255).Get();
					}
					else if (p->shield_max == 75)
					{
						_Color = ImColor(0, 191, 255).Get();
					}
					else if (p->shield_max == 100)
					{
						_Color = ImColor(153, 50, 204).Get();
					}
					else if (p->shield_max == 125)
					{
						_Color = ImColor(255, 0, 0).Get();
					}
				}

				float health_X = startx - 18;
				float health_Y = starty;
				float health_H = h;
				float String_X = startx;
				float String_Y = starty + h;
				// float health_W = w * 2;
				// health_X = health_X - (w / 2);
				float health_length = (health_H - 2) * scale;
				float shield_length = (health_H - 2) * shield_scale;
				// 绘制护盾
				ctx->AddBox(health_X + 1, health_Y + 1, 2, shield_length, _Color);
				ctx->AddBox(health_X + 2, health_Y + 1, 2, shield_length, _Color);
				// ctx->AddLine(lx, ly, lx, starty + lh, ImColor(0x3E, 0xBD, 0).Get());

				// 护盾边框
				// ctx->AddBox(health_X, health_Y, 5, health_H, ImColor(255, 255, 255).Get());
				// ctx->FillRect(0, 0, 50, 50, ImColor(58, 208, 45).Get());

				// 绘制血条
				ctx->AddBox(health_X + 9, health_Y + 1, 2, health_length, ImColor(62, 189, 0).Get());
				ctx->AddBox(health_X + 10, health_Y + 1, 2, health_length, ImColor(62, 189, 0).Get());
				// ctx->AddLine(lx, ly, lx, starty + lh, ImColor(0x3E, 0xBD, 0).Get());

				// 血条边框
				// ctx->AddBox(health_X + 8, health_Y, 5, health_H, ImColor(255, 255, 255).Get());
				// 绘制队标
				// std::string _str = "[" + std::to_string(p->TeamNumber) + "]";
				// const char *Str = _str.c_str();
			}
		}
	}
}

void DrawAll()
{
	rendsender *ctx = m_backend->rend();
	ctx->Begin();

	// ctx->FillRect(50, 50, 100, 100, ImColor(255, 255, 255).Get());

	// const char *Str = "sssssssssss";
	// ctx->AddString(100, 100, Str, 12, ImColor(255, 255, 255).Get());

	DrawPlayers(ctx);
	if (Vars::AimPlayer)
	{
		vec2_t center;
		center.x = m_width / 2;
		center.y = m_height / 2;
		vec3_t spos = {};
		if (w2s(Vars::AimPlayer->aimpart, spos))
		{
			ctx->AddLine(center.x, center.y, spos.x, spos.y, ImColor(255, 255, 255).Get());
		}
	}
	ctx->PresentToTask();
}
void SetScreen(float w, float h)
{
	m_width = w;
	m_height = h;
	m_aim.setResolution(w, h);
}

ULONG64 first_smart_time = 0;
ULONG64 first_s_time = 0;
void UpdateSmartAim()
{
	return;
}

bool UpdateLocalPlayer(USHORT id)
{
	// ULONG64 LocalPlayer = *(ULONG64*)(GlobalObjectCache + 0x20 * id);
	// if (!LocalPlayer)
	//	return false;
	ULONG64 LocalPlayer = mem::r<ULONG64>(Vars::pGameImage + offsets::local_player);
	USHORT weapid = mem::r<USHORT>(LocalPlayer + offsets::latestPrimaryWeapons);
	// _ptb("weapid:%d\r\n", weapid);
	if (weapid)
	{
		ULONG64 pWeap = *(ULONG64 *)(GlobalObjectCache + 0x20 * weapid);
		// p1x(pWeap);
		if (pWeap && MmiGetPhysicalAddress((PVOID)pWeap))
		{
			Vars::LocalPlayer = LocalPlayer;
			Vars::LocalWeapId = weapid;
			Vars::LocalWeap = pWeap;
			Vars::LocalTeamId = mem::r<USHORT>(LocalPlayer + offsets::TeamNumber);
			Vars::LocalViewAngle = mem::r<vec3_t>(LocalPlayer + offsets::view_angles);
			Vars::LocalPlayerPos = mem::r<vec3_t>(LocalPlayer + offsets::base_pos);
			Vars::LocalPlayerCameraPos = mem::r<vec3_t>(LocalPlayer + offsets::CameraPos);
			Vars::LocalAimPunch = mem::r<vec3_t>(LocalPlayer + offsets::aim_punch);
			Vars::LocalDynamicAngle = mem::r<vec3_t>(LocalPlayer + offsets::DynamicAngle);
			Vars::LocalWeapSpeed = mem::r<vec3_t>(pWeap + offsets::bullet_velocity);

			// _ptb("LocalWeapSpeed.z:%d\r\n", (int)Vars::LocalWeapSpeed.z);

			float temp = Vars::LocalViewAngle.x;
			Vars::LocalViewAngle.x = Vars::LocalViewAngle.y;
			Vars::LocalViewAngle.y = temp;

			temp = Vars::LocalDynamicAngle.x;
			Vars::LocalDynamicAngle.x = Vars::LocalDynamicAngle.y;
			Vars::LocalDynamicAngle.y = temp;

			return true;
		}
	}

	return false;
}
bool ParsePlayer(Player *p)
{
	if (Vars::BoneBufferIndex >= MAX_PLAYER)
		return false;
	if (p->pObject)
	{
		ULONG64 obj = p->pObject;
		// p->EntityId = id;
		p->pos = mem::read2<vec3_t>(obj + offsets::base_pos);
		p->health = mem::read2<int>(obj + offsets::health);
		p->health_max = mem::read2<int>(obj + offsets::health_max);

		p->TeamNumber = mem::read2<USHORT>(obj + offsets::TeamNumber);
		if (p->TeamNumber == Vars::LocalTeamId)
			return false;

		p->shield = mem::read2<int>(obj + offsets::shield);
		p->shield_max = mem::read2<int>(obj + offsets::shield_max);
		// _ptb("shield:%d\r\n", p->shield);
		// _ptb("shield_max:%d\r\n", p->shield_max);
		p->velocity = mem::read2<vec3_t>(obj + offsets::velocity);

		p->life_state = mem::read2<ULONG>(obj + offsets::bleedoutState);
		p->vistime = mem::read2<float>(obj + offsets::VisibleTime);
		p->obsered = mem::read2<USHORT>(obj + offsets::ObserverTarget);

		if (p->obsered != 0 && p->obsered != (USHORT)-1)
		{
			if (p->obsered == Vars::LocalPlayerId)
				if (p->TeamNumber != Vars::LocalTeamId)
					Vars::m_uObseredCount++;
		}

		ULONG64 pBoneMatrix = mem::read2<ULONG64>(obj + offsets::bones);
		if (pBoneMatrix)
		{
			PUCHAR pBoneBuffer = Vars::BoneBuffer + Vars::BoneBufferIndex * BONE_BUFFER_MAXSIZE;
			MmiReadVirtualAddressSafe(pBoneMatrix + 0xC0, pBoneBuffer, BONE_BUFFER_MAXSIZE);
			p->OffBoneBuffer = 1 + Vars::BoneBufferIndex * BONE_BUFFER_MAXSIZE;
			Vars::BoneBufferIndex++;
		}

		p->head = get_bone(p, 7);
		p->chest = get_bone(p, 1);

		p->aimpart = p->head;
		if (Flag::user_aimbot_state)
		{
			if (Flag::user_aimbot_state == 2)
			{
				p->aimpart = get_bone(p, 5);
			}
			if (Flag::user_aimbot_state == 3)
			{
				p->aimpart = get_bone(p, 1);
			}
		}

		p->aimpos = calc_aimbot_pos(p->aimpart, p->velocity);
		p->viewoffset = calc_viewoffset(p->chest);
		p->view_distance = p->viewoffset.len();

		float dis = p->pos.dis(Vars::LocalPlayerPos) / 10.f;
		p->dis_to_local = dis;
		bool vaild_draw = false;

		if (p->health && (dis <= Flag::user_ESP_distance))
		{
			vaild_draw = true;
		}
		else
		{
			vaild_draw = false;
		}

		p->vaild_aimbot = vaild_draw;
		if (p->vaild_aimbot)
		{
			if (p->health <= 0 || (dis > Flag::user_ESP_distance))
			{
				p->vaild_aimbot = false;
			}
			if (p->life_state != 0)
			{
				if (Flag::user_aimbot_notdown)
				{
					p->vaild_aimbot = false;
				}
			}
		}
		if (p->view_distance > Flag::user_Aimbot_Range)
		{
			p->inrange = false;
		}
		else
		{
			p->inrange = true;
		}

		return true;
	}
	return false;
}
void ParseAimPlayer()
{
	static bool aimbot_havelast = 0;
	static int aimbot_lastid = 0;

	size_t size = Vars::PlayerCount;
	if (Flag::Aimbot)
	{
		if (aimbot_havelast)
		{
			for (size_t i = 0; i < size; i++)
			{
				Player *p = &Vars::PlayerList[i];
				if (p->vaild_aimbot)
				{
					if (p->EntityId == aimbot_lastid)
					{
						Vars::AimPlayer = p;
						return;
					}
				}
			}
		}
	}
	if (Flag::user_aimbot_continue == false)
	{
		if (Flag::Aimbot)
		{
			if (aimbot_havelast)
			{
				Flag::Aimbot = false;
			}
		}
	}

	// 重新寻找
	aimbot_havelast = false;
	aimbot_lastid = 0;

	float _min = 0.f;
	Player *_min_player = 0;
	bool first = false;
	for (size_t i = 0; i < size; i++)
	{
		Player *p = &Vars::PlayerList[i];
		// std::cout << "vaild_aimbot " << p->vaild_aimbot << std::endl;
		if (p->vaild_aimbot && p->inrange)
		{
			if ((first == false) || (p->view_distance < _min))
			{
				first = true;
				_min = p->view_distance;
				_min_player = p;
			}
		}
	}
	if (_min_player)
	{
		Vars::AimPlayer = _min_player;
		aimbot_lastid = Vars::AimPlayer->EntityId;
		aimbot_havelast = true;
	}
}
void UpdateVisibleInformation()
{
	for (int i = 0; i < Vars::PlayerCount; i++)
	{
		Player *p = &Vars::PlayerList[i];
		ULONG id = p->EntityId;
		float time = p->vistime;
		float cacheTime = GlobalVisibleTimeCache[id];
		if (time > 0.f && cacheTime > 0.f && time > cacheTime)
		{
			GlobalVisibleCache[id] = true;
		}
		else
		{
			GlobalVisibleCache[id] = false;
		}
		GlobalVisibleTimeCache[id] = time;
	}
}
void ParseVisibleInfomation()
{
	for (int i = 0; i < Vars::PlayerCount; i++)
	{
		Player *p = &Vars::PlayerList[i];

		if (p->vistime > 0.f)
		{
			p->visible = GlobalVisibleCache[p->EntityId];
		}
	}
}
VOID UpdateMatrix(float *mat)
{
	memcpy(&m_mat, mat, 64);
	// Aim::UpdateMatrix(mat);

	return;
}
extern VOID ProcessHotKeys();
bool UpdateData()
{
	Vars::BoneBufferIndex = 0;
	Vars::LocalPlayer = 0;
	Vars::LocalPlayerId = 0;
	Vars::LocalWeap = 0;

	Vars::LocalTeamId = 0;
	Vars::AimPlayer = 0;
	Vars::m_uObseredCount = 0;

	// _ptb("update!");

	// USHORT id = mem::r<USHORT>(Vars::pGameImage + offsets::local_player_id);
	// if (id == 0)
	//	return;
	//_ptb("local_player_id:%d", id);
	// Vars::LocalPlayerId = id;
	bool result = false;

	static ULONG64 latestUpdateTime = 0;

	if (mem::read(Vars::pGameImage + offsets::entity_list, GlobalObjectCache, 0x20 * 0x10000))
	{
		if (GetRealTime() - latestUpdateTime > 1000)
		{
			Vars::PlayerCount = 0;
			RtlZeroMemory(Vars::PlayerList, sizeof(Vars::PlayerList));
			// _ptb("Read Success!");
			if (!UpdateLocalPlayer(0))
				return false;
			_ptb("LocalPlayer Success!");
			for (int i = 0; i < 0x10000; i++)
			{
				ULONG64 pObject = *(ULONG64 *)(GlobalObjectCache + 0x20 * i);
				if (!pObject)
					continue;
				ULONG64 pName = mem::r<ULONG64>(pObject + offsets::iSignifierName);
				ULONG64 v = mem::r<ULONG64>(pName);
				LARGE_INTEGER l;
				l.QuadPart = v;
				if ((l.LowPart == 'yalp' && (l.HighPart & 0xFFFFFF) == '\0re') || (l.LowPart == '_cpn' && (l.HighPart & 0xFFFFFF) == 'mud'))
				{
					if (Vars::PlayerCount < MAX_PLAYER)
					{
						Player p;
						p.pObject = pObject;
						p.EntityId = i;
						if (pObject != Vars::LocalPlayer && ParsePlayer(&p))
						{
							Vars::PlayerList[Vars::PlayerCount++] = p;
						}
					}
				}
			}
			latestUpdateTime = GetRealTime();
		}
		else
		{
			{
				if (!UpdateLocalPlayer(0))
					return false;
				ULONG64 objs[100];
				ULONG64 ids[100];
				for (int i = 0; i < Vars::PlayerCount; i++)
				{
					Player *p = &Vars::PlayerList[i];
					objs[i] = p->pObject;
					ids[i] = p->EntityId;
				}
				int oldcount = Vars::PlayerCount;
				Vars::PlayerCount = 0;
				RtlZeroMemory(Vars::PlayerList, sizeof(Vars::PlayerList));
				for (int i = 0; i < oldcount; i++)
				{
					Player p;
					p.pObject = objs[i];
					p.EntityId = ids[i];
					if (p.pObject != Vars::LocalPlayer && ParsePlayer(&p))
					{
						Vars::PlayerList[Vars::PlayerCount++] = p;
					}
				}
			}
		}

		ULONG64 ViewRender = mem::r<ULONG64>(Vars::pGameImage + offsets::ViewRender);
		if (ViewRender)
		{
			ULONG64 pMatrix = mem::r<ULONG64>(ViewRender + offsets::Matrix);
			if (pMatrix)
			{
				// DbgPrint("[Help] pMatrix:%p\n", pMatrix);
				float mat[16] = {0};
				mem::read(pMatrix, mat, 64);
				UpdateMatrix(mat);
			}
		}
		// printf("[Help] Player count: %d\n", Vars::PlayerCount);
		{
			static SHORT exec_lock = 0;
			static ULONG64 exec_last_time = 0;

			if (InterlockedCompareExchange16(&exec_lock, 1, 0) == 0)
			{
				ULONG64 now_time = GetRealTime();
				int detal_time = 50;
				if (now_time - exec_last_time > detal_time)
				{
					exec_last_time = now_time;
					UpdateVisibleInformation();
				}
				exec_lock = 0;
			}
		}
		ParseVisibleInfomation();

		ParseAimPlayer();
		if (Vars::AimPlayer)
		{
			printf("[Help] AimPlayerId:%d\n", Vars::AimPlayer->EntityId);
		}
		UpdateSmartAim();

		bool isEnableAim = false;
		if (Flag::user_aimbot_on)
		{
			if (Vars::AimPlayer)
			{
				if (Flag::Aimbot && Vars::AimPlayer->visible)
				{
					isEnableAim = true;
					printf("[Help] Aiming\n");
					result = true;
				}
			}
		}
		if (isEnableAim)
		{
			// Aim::UpdateDriverAimTarget(Vars::AimPlayer->aimpos);
			vec3_t aim_screen;
			if (w2s(Vars::AimPlayer->aimpos, aim_screen))
			{
				m_aim.updateTargetPosition(aim_screen.x, aim_screen.y);
			}
			// EnableDriverAim();
		}
		else
		{
			// DisableDriverAim();
		}
	}
	return result;
}
void UpdateDataDisableSmap()
{
	DispatchHotkey();
	if (UpdateData())
	{
		// Aim::EnableDriverAim();
		m_aim.enableAim();
	}
	else
	{
		// Aim::DisableDriverAim();
		m_aim.disableAim();
	}
	DrawAll();
}

void LaunchAimThread()
{
	m_aim.setFunction([](int x, int y)
					  { m_backend->input_mouse_move(x, y); })
		.setRate(100)
		.setResolution(m_backend->screen_width(), m_backend->screen_height())
		.launchThread();
}
void CancelAimThread()
{
	m_aim.cancelThread();
}
