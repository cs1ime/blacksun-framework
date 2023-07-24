#pragma once

#ifndef _LAYER_H_
#define _LAYER_H_

#include <dma.h>
#include "windows.h"
#include "util.h"

namespace Apex {
	extern std::shared_ptr<process> g_apex;
}

namespace smem {
	template <typename T>
	bool read(u64 addr, T* result) {
		return Apex::g_apex->read_virt(addr, result, sizeof(T));
	}
	bool read_string(const char* addr, ptr buffer, u64 sz);
	bool read_data(PVOID addr, ptr buffer, u64 sz);
}
namespace mem {
	extern mmu_cache c;
	template <typename T>
	void w(ULONG64 addr, T data) {
		Apex::g_apex->w<T>(addr, data);
	}
	template <typename T>
	T r(ULONG64 addr) {
		return Apex::g_apex->r<T>(addr);
	}
	template <typename T>
	T read2(ULONG64 addr) {
		T result;
		memset(&result, 0, sizeof(result));
		Apex::g_apex->read_with_cache(&c, addr, &result, sizeof(result));
		return result;
	}
	bool read(ULONG64 addr, PVOID buf, u64 size);

}
ULONG64 AsmRdtsc();
#define GetRealTime util::GetTickCount

u64 MmiGetPhysicalAddress(ptr addr);
bool MmiReadVirtualAddressSafe(u64 addr, ptr result, u64 sz);
u64 ScanMemory(u64 addr, u64 sz, const char* pat);

#endif // !_LAYER_H_

