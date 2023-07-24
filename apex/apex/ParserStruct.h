#pragma once

#ifndef _PARSER_STRUCT_H
#define _PARSER_STRUCT_H
#include "windows.h"
#include "vector.h"
#include "Vars.h"

struct Player {
	ULONG64 pObject;
	USHORT EntityId;

	vec3_t pos;
	vec3_t head;
	vec3_t chest;
	int health;
	int health_max;
	int shield;
	int shield_max;
	ULONG life_state;
	ULONG TeamNumber;
	vec3_t velocity;
	ULONG OffBoneBuffer;
	USHORT obsered;
	ULONG kill;
	float vistime;

	bool vaild_aimbot;
	bool inrange;
	bool visible;
	float view_distance;

	vec3_t aimpart;
	vec3_t aimpos;
	vec2_t viewoffset;
	float dis_to_local;

};

#endif // !_PARSER_STRUCT_H

