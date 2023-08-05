#include "CompatibleLayer.h"
#include "windows.h"
#include "dma.h"


namespace smem {
	bool read_string(const char* addr, ptr buffer, ULONG64 sz)
	{
		return Apex::g_apex->read_string((ULONG64)addr, (pstr)buffer, sz);
	}
	bool read_data(PVOID addr, ptr buffer, u64 sz) {
		return Apex::g_apex->read_virt((u64)addr, buffer, sz);
	}
}
namespace mem {
	mmu_cache c;
	bool read(ULONG64 addr, PVOID buf, u64 size) {
		return Apex::g_apex->read_virt(addr, buf, size);
	}
}
ULONG64 MmiGetPhysicalAddress(ptr addr) {
	u64 uaddr = Apex::g_apex->virt2phys((u64)addr);
	return uaddr == -1 ? 0 : uaddr;
}
bool MmiReadVirtualAddressSafe(DWORD64 addr, PVOID result, u64 sz) {
	return Apex::g_apex->read_virt(addr, result, sz);
}

ULONG64 ScanMemory(ULONG_PTR addr, ULONG64 sz, LPCSTR pattern) {
	u64 result = Apex::g_apex->scanmem(addr, addr + sz, pattern);
	return result == -1 ? 0 : addr + result;
}


