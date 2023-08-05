
#include "NetVars.h"
#include "CompatibleLayer.h"
#include "oldnames.h"
#include <map>

namespace CNetVars {
	ULONG64 m_tables[10000];
	int m_tables_idx = 0;
	void init_m_tables() {
		m_tables_idx = 0;
	}
	void Initialize(ULONG64 NetVarPtr)
	{

		m_tables_idx = 0;

		ULONG64 clientClass = NetVarPtr;
		if (!clientClass)
			return;

		while (clientClass)
		{
			client_class cl;
			RtlZeroMemory(&cl, sizeof(cl));
			if (!smem::read<client_class>(clientClass, &cl))break;
			ULONG64 recv_table = cl.table;
			m_tables[m_tables_idx++] = recv_table;
			if (m_tables_idx >= 10000)
				return;
			//DbgPrint("[112233] i:%d first:%p\n", m_tables_idx, recv_table);
			clientClass = cl.next;
		}

	}

	std::map<ULONG64,std::map<std::string,int>> g_map;
	int PrintAll_Prop(ULONG64 recvtable, const char* propName, int ins)
	{
		if(propName)
		{
			if(g_map.find(recvtable) != g_map.end())
			{
				auto & map = g_map[recvtable];
				if(map.find(propName)!=map.end())
				{
					return map[propName];
				}
			}
		}
		if (ins >= 10)
			return 0;
		ULONG numProps = 0;
		ULONG64 props = 0;
		if (!smem::read<ULONG>(recvtable + offsetof(recv_table,num_props), &numProps))return 0;
		if (!smem::read<ULONG64>(recvtable + offsetof(recv_table,props), &props))return 0;
		int extraOffset = 0;
		for (int i = 0; i < numProps; ++i)
		{
			ULONG64 recv_pr = 0;
			if (!smem::read<ULONG64>(props + i * 8, &recv_pr))continue;
			recv_prop rp;
			RtlZeroMemory(&rp, sizeof(rp));
			if (!smem::read<recv_prop>(recv_pr, &rp))continue;

			ULONG64 child = rp.data_table;
			ULONG cnumProps = 0;
			if (smem::read<ULONG>(child + offsetof(recv_table,num_props), &cnumProps)) {
				if (child && (cnumProps > 0))
				{
					int tmp = PrintAll_Prop(child, propName, ins + 1);
					if (tmp)
						extraOffset += (rp.offset + tmp);
				}
			}
			CHAR Name[100];
			RtlZeroMemory(Name, sizeof(Name));
			if (!smem::read_string((const char*)rp.name, Name, 80)) {
				continue;
			}
			if(g_map.find(recvtable)==g_map.end())
			{
				g_map.insert({recvtable,{{}}});
			}
			auto &map=g_map[recvtable];
			map.insert({Name,(rp.offset + extraOffset)});
			if(propName)
			{
				if (stricmp(Name, propName))
					continue;
				return (rp.offset + extraOffset);
			}
		}

		return extraOffset;
	}
	
	void PrintAll(ULONG64 NetVarPtr)
	{
		m_tables_idx = 0;

		ULONG64 clientClass = NetVarPtr;
		if (!clientClass)
			return;

		while (clientClass)
		{
			client_class cl;
			RtlZeroMemory(&cl, sizeof(cl));
			if (!smem::read<client_class>(clientClass, &cl))break;
			ULONG64 recv_table = cl.table;
			m_tables[m_tables_idx++] = recv_table;
			if (m_tables_idx >= 10000)
				return;
			clientClass = cl.next;
		}

		for (int i = 0; i < m_tables_idx; i++) {
			ULONG64 table = m_tables[i];
			//DbgPrint("[112233] table:%p\n", table);
			if (!table)
				continue;
			recv_table t;
			RtlZeroMemory(&t, sizeof(t));
			if (smem::read<recv_table>(table, &t)) {
				//DbgPrint("[112233] table:%p\n", table);
				CHAR Name[100];
				RtlZeroMemory(Name, sizeof(Name));
				if (smem::read_string((const char*)t.name, Name, 80)) {
					//DbgPrint("[112233] Name:%s\n", Name);
					PrintAll_Prop(table,0,0);
					if(g_map.find(table)!=g_map.end())
					{
						auto &map=g_map[table];
						for (auto& [propName,Offset] : map)
						{
							printf("klass:%-40s name:%-50s offset:0x%-08llX\r\n",Name,propName.c_str(),Offset);
						}
					}
				}
			}

		}

	}

	int GetOffset(const char* tableName, const char* propName)
	{
		int offset = Get_Prop1(tableName, propName, 0);
		if (!offset)
		{
			return 0;
		}
		return offset;
	}
	int Get_Prop1(const char* tableName, const char* propName, ULONG64 prop)
	{

		ULONG64 recv_table = GetTable(tableName);
		if (!recv_table)
			return 0;

		int offset = Get_Prop(recv_table, propName, prop);
		if (!offset)
			return 0;

		return offset;
	}
	//bool ResolveOffset(,)
	int Get_Prop(ULONG64 recvtable, const char* propName, ULONG64 prop, int ins)
	{
		if (ins >= 10)
			return 0;
		ULONG numProps = 0;
		ULONG64 props = 0;
		if (!smem::read<ULONG>(recvtable + offsetof(recv_table,num_props), &numProps))return 0;
		if (!smem::read<ULONG64>(recvtable + offsetof(recv_table,props), &props))return 0;
		int extraOffset = 0;
		for (int i = 0; i < numProps; ++i)
		{
			ULONG64 recv_pr = 0;
			if (!smem::read<ULONG64>(props + i * 8, &recv_pr))continue;
			recv_prop rp;
			RtlZeroMemory(&rp, sizeof(rp));
			if (!smem::read<recv_prop>(recv_pr, &rp))continue;

			ULONG64 child = rp.data_table;
			ULONG cnumProps = 0;
			if (smem::read<ULONG>(child + offsetof(recv_table,num_props), &cnumProps)) {
				if (child && (cnumProps > 0))
				{
					int tmp = Get_Prop(child, propName, prop, ins + 1);
					if (tmp)
						extraOffset += (rp.offset + tmp);
				}
			}
			CHAR Name[100];
			RtlZeroMemory(Name, sizeof(Name));
			if (!smem::read_string((const char*)rp.name, Name, 80)) {
				continue;
			}
			if (stricmp(Name, propName))
				continue;
			return (rp.offset + extraOffset);
		}

		return extraOffset;
	}
	ULONG64 GetTable(const char* tableName)
	{
		for (int i = 0; i < m_tables_idx; i++) {
			ULONG64 table = m_tables[i];
			if (!table)
				continue;
			recv_table t;
			RtlZeroMemory(&t, sizeof(t));
			if (smem::read<recv_table>(table, &t)) {
				CHAR Name[100];
				RtlZeroMemory(Name, sizeof(Name));
				if (smem::read_string((const char*)t.name, Name, 80)) {
					if (!stricmp(Name, tableName))
						return table;
				}
			}

		}

		return 0;
	}

}
