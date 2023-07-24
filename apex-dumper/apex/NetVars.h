#pragma once

#ifndef _NETVARS_H_
#define _NETVARS_H_

#include "windows.h"

struct recv_prop
{
	ULONG type; //0x0000
	ULONG offset; //0x0004
	CHAR pad_0008[24]; //0x0008
	ULONG64 data_table; //0x0020
	ULONG64 name; //0x0028
	bool is_inside_array; //0x0030
	CHAR pad_0031[7]; //0x0031
	ULONG64 array_prop; //0x0038
	ULONG64 proxy_fn; //0x0040
	CHAR pad_0048[12]; //0x0048
	ULONG flags; //0x0054
	CHAR pad_0058[4]; //0x0058
	ULONG num_elements; //0x005C
}; //Size: 0x0060
struct recv_table
{
	char pad_0000[8]; //0x0000
	ULONG64 props; //0x0008
	ULONG num_props; //0x0010
	char pad_0014[1196]; //0x0014
	ULONG64 decoder; //0x04C0
	ULONG64 name; //0x04C8
	//bool initialized; //0x04D0
	//bool in_main_list; //0x04D1
}; //Size: 0x04D2
struct client_class
{
	ULONG64 create_fn; //0x0000
	ULONG64 create_event_fn; //0x0008
	ULONG64 network_name; //0x0010
	ULONG64 table; //0x0018
	ULONG64 next; //0x0020
	ULONG id; //0x0028
	char pad_002C[4]; //0x002C
	ULONG64 name; //0x0030
}; //Size: 0x0038

namespace CNetVars {
	void init_m_tables();
	void Initialize(ULONG64 NetVarPtr);
	int GetOffset(const char* tableName, const char* propName);
	int Get_Prop1(const char* tableName, const char* propName, ULONG64 prop = 0);
	int Get_Prop(ULONG64 recvtable, const char* propName, ULONG64 prop, int ins = 0);
	ULONG64 GetTable(const char* tableName);
	void PrintAll(ULONG64 NetVarPtr);
};



#endif // !_NETVARS_G_


