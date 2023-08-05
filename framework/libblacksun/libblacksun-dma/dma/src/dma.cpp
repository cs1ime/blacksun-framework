#include "../include/dma.h"
#include "../include/dma_mmu.h"
#include "../include/dma_ntutil.h"
#include "../include/dma_symbol.h"
#include "windows_defs.h"
#include <iostream>
#include <map>

#include "src_olddefine.h"

#define printf

std::shared_ptr<ntfuncs> ntfunc_creator::try_create() {
  physaddr root_tb = dma_find_root_tb();
  printf("dtb:%llX\n", root_tb);
  if (root_tb == 0)
    return 0;
  u8 root_tb_data[0x1000] = {};
  read_physical_memory(root_tb, root_tb_data, 0x1000);
  uptr pte_base = dma_find_selfmapping_ptebase(root_tb_data, root_tb);
  printf("pte_base:%llX\n", pte_base);
  if (pte_base == 0)
    return nullptr;
  g_sysmmu =
      std::make_shared<mmu>(mmu_initializer(root_tb, pte_base, m_accessor));
  uptr pfnbase = dma_find_MmPfnDataBase_from_dtb(root_tb_data);
  printf("pfnbase:%llX\n", pfnbase);
  if (pfnbase == 0)
    return nullptr;
  g_root_tb = root_tb;
  g_ptebase = pte_base;
  g_pfnbase = pfnbase;
  mmu_initializer _m(root_tb, pte_base, m_accessor);
  u64 ntbase = dma_find_ntoskrnl(root_tb);

  auto funcs = std::make_shared<ntfuncs>(_m, ntbase, (m_symbol_factory));
  if (!funcs->is_valid()) {
    return nullptr;
  }

  return funcs;
}
physaddr ntfunc_creator::mmu_get_pa_from_dtb(physaddr dtb, uptr va) {
  hw_pte PageEntry[3] = {0};
  hw_pte page;
  va_t vva;
  vva.all = (u64)va;
  read_physical_memory(dtb + vva.pml4e_index * 8, (pbyte)&page.all, 8);
  if (page.present == 0 || page.large_page == 1)
    return D_BADPHYSADDR;
  read_physical_memory((page.page_frame_number << 12) + vva.pdpte_index * 8,
                       (pbyte)&page.all, 8);
  if (page.present == 0)
    return badphysaddr;
  if (page.large_page) {
    u64 off = (u64)va & 0x3FFFFFFF;
    u64 PhysAddr = page.page_frame_number << 12;
    PhysAddr += off;
    return PhysAddr;
  }
  read_physical_memory((page.page_frame_number << 12) + vva.pde_index * 8,
                       (pbyte)&page.all, 8);
  if (page.present == 0)
    return badphysaddr;
  if (page.large_page) {
    u64 off = (u64)va & 0x1FFFFF;
    u64 PhysAddr = page.page_frame_number << 12;
    PhysAddr += off;
    return PhysAddr;
  }
  read_physical_memory((page.page_frame_number << 12) + vva.pte_index * 8,
                       (pbyte)&page.all, 8);
  if (page.present == 0)
    return badphysaddr;
  u64 off = (u64)va & 0xFFF;
  u64 PhysAddr = page.page_frame_number << 12;
  PhysAddr += off;
  return PhysAddr;
}
uptr ntfunc_creator::dma_find_MmPfnDataBase_from_dtb(u8 *pb) {
  for (int i = 0x100; i < 0x200; i++) {
    u64 dtb_offset = (g_sysmmu->get_dtb() >> 12) * 0x30;
    va_t root_va;
    root_va.all = 0;
    root_va.pml4e_index = i;
    uptr pmmpfn = dtb_offset + root_va.all;
    u64 pte_addr = 0;
    if (g_sysmmu->read_virt(pmmpfn + 0x8, (u8 *)&pte_addr, 8)) {
      va_t va_pte_base;
      va_pte_base.all = g_sysmmu->get_pml4e_base() & 0x0000FFFFFFFFF000;
      va_pte_base.offset = 8 * va_pte_base.pml4e_index;
      va_pte_base.head = 0xFFFF;
      u64 dtb_pteaddr = va_pte_base.all;
      if (dtb_pteaddr == pte_addr) {
        va_t va_mmpfndatabase = {0};
        va_mmpfndatabase.pml4e_index = i;
        return va_mmpfndatabase.all;
      }
    }
  }
  return 0;
}
uptr ntfunc_creator::dma_find_selfmapping_ptebase(u8 *pb, physaddr pa) {
  pqword pq = (pqword)pb;
  for (int i = 0; i < 0x100; i++) {
    u64 val = pq[i + 0x100];
    hw_pte phys = {};
    phys.all = val;
    u64 pfn = ((u64)pa) >> 12;

    if (phys.page_frame_number == pfn) {
      va_t va = {};
      va.all = 0;
      va.head = 0xFFFF;
      va.pml4e_index = i + 0x100;
      return va.all;
    }
  }
  return 0;
}
#include "../include/dma_type.h"
bool ntfunc_creator::dma_check_root_tb(u8 *pb, physaddr pa) {
  auto dma_check_valid_buildnumber = [&](int buildnumber) -> bool {
    return buildnumber >= 7600 && buildnumber < 0x1000000;
    // switch (buildnumber)
    // {
    // case 7600:
    // case 7601:
    // case 9200:
    // case 9600:
    // case 10240:
    // case 10586:
    // case 14393:
    // case 15063:
    // case 16299:
    // case 17134:
    // case 17763:
    // case 18362:
    // case 18363:
    // case 19041:
    // case 19042:
    // case 19043:
    // case 19044:
    // case 19045:
    // case 20348:
    // case 22000:
    // case 22621:
    // 	return true;
    // default:
    // 	break;
    // }
    // return false;
  };
  auto dma_check_root_tb_judgement1 = [&](pbyte pb, physaddr pa) -> bool {
    return dma_find_selfmapping_ptebase(pb, pa) != 0;
  };
  auto dma_check_root_tb_judgement2 = [&](u8 *pb, physaddr pa) -> bool {
    u64 rpa = mmu_get_pa_from_dtb(pa, 0xFFFFF78000000000);
    if (rpa == D_BADPHYSADDR)
      return false;
    printf("shareddatapa:%llX\n", rpa);
    u8 shared_data[0x1000] = {};
    if (!read_physical_memory(rpa, shared_data, 0x1000))
      return false;
    u32 buildnumber = *(u32 *)(shared_data + 0x260);
    printf("buildnumber:%d\n", buildnumber);
    if (!dma_check_valid_buildnumber(buildnumber))
      return false;
    u64 ret_inst = *(u64 *)(shared_data + 0x2f8);
    printf("ret_inst:%llX\n", ret_inst);
    if (ret_inst != 0xC3)
      return false;
    return true;
  };

  if (dma_check_root_tb_judgement1(pb, pa) &&
      dma_check_root_tb_judgement2(pb, pa))
    return true;
  return false;
}
physaddr ntfunc_creator::dma_find_root_tb() {
  for (physaddr pa = 0; pa < 0x1000000; pa += PAGE_SIZE) {
    // printf("pa:%llX\n",pa);
    u8 data[0x1000] = {};
    if (read_physical_memory(pa, data, 0x1000)) {
      if (dma_check_root_tb(data, pa)) {
        printf("%llX\n", pa);
        return pa;
      }
    }
  }
  return 0;
}
u64 ntfunc_creator::dma_find_ntoskrnl(physaddr dtb) {
  va_t start_va;
  start_va.all = 0xF80000000000;
  va_t end_va;
  end_va.all = 0xF807FFFFFFFF;
  u64 ntos = 0;
  g_sysmmu->enummem(
      start_va.all, end_va.all, [&](u64 start, u64 sz, u64 rwx) -> bool {
        printf("%llX %llX\n", start, sz);
        printf("phys: %llX\n", g_sysmmu->virt2phys(start));
        u64 pagecount = sz / 0x1000;
        byte pagedata[0x1000];
        word mz = 0;
        for (u64 i = 0; i < pagecount; i++) {
          if (g_sysmmu->r<WORD>(start + i * 0x1000) == 0x5A4D) {
            printf("mz!\n");
            u64 hdr = start + i * 0x1000;
            IMAGE_DOS_HEADER *idh = (IMAGE_DOS_HEADER *)hdr;
            LONG e_lfanew = 0;
            e_lfanew = g_sysmmu->r<LONG>(&idh->e_lfanew);
            if (e_lfanew > 0xA00)
              continue;
            IMAGE_NT_HEADERS *inh = (IMAGE_NT_HEADERS *)(hdr + e_lfanew);
            int magic = g_sysmmu->r<int>(inh);
            if (magic == 0x00004550) {
              printf("pe!\n");
              IMAGE_SECTION_HEADER *ish =
                  (IMAGE_SECTION_HEADER *)(((pbyte)&inh->OptionalHeader) +
                                           g_sysmmu->r<WORD>(
                                               &inh->FileHeader
                                                    .SizeOfOptionalHeader));

              WORD sectcnt =
                  g_sysmmu->r<WORD>(&inh->FileHeader.NumberOfSections);
              for (int j = 0; j < sectcnt; j++) {
                IMAGE_SECTION_HEADER *curish = &ish[j];
                char sectname[9];
                memset(sectname, 0, 9);
                *(u64 *)(sectname) = g_sysmmu->r<u64>(&curish->Name);
                if (*(DWORD *)sectname == 0x4C4F4F50 &&
                    *(DWORD *)(sectname + 4) == 0x45444F43) {
                  printf("finded!\n");
                  ntos = hdr;
                  return false;
                }
              }
            }
          }
        }
        return true;
      });
  return ntos;
}
