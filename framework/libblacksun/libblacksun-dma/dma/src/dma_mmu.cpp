#include "../include/dma.h"
#include "../include/dma_mmu.h"
#include <iostream>
#include <unordered_map>
#include "oldnames_inline.h"
#include <string.h>
#include "src_olddefine.h"
#include <list>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <utility>
#include <functional>

using std::pair;
using std::unordered_map;

mmu_tlb::mmu_tlb(const mmu_initializer &_m) : mmu_initializer(_m)
{
    mmu_tlb_4k.clear();
    mmu_tlb_2m.clear();
    mmu_tlb_1g.clear();
}
void mmu_tlb::invalidate()
{
    smtx.lock();
    mmu_tlb_4k.clear();
    mmu_tlb_2m.clear();
    mmu_tlb_1g.clear();
    smtx.unlock();
}

mmu_pagetype mmu_tlb::entryexist(uptr virt)
{
    u64 vpn = (virt & 0x0000FFFFFFFFF000);
    if (this->mmu_tlb_4k.find(vpn) != this->mmu_tlb_4k.end())
        return MMU_PAGE_4K;
    vpn = (virt & 0x0000FFFFFFE00000);
    if (this->mmu_tlb_2m.find(vpn) != this->mmu_tlb_2m.end())
        return MMU_PAGE_2M;
    vpn = (virt & 0x0000FFFFC0000000);
    if (this->mmu_tlb_1g.find(vpn) != this->mmu_tlb_1g.end())
        return MMU_PAGE_1G;
    return MMU_PAGE_INVALID;
}
physaddr mmu_tlb::entryget(uptr virt)
{
    physaddr result = badphysaddr;
    smtx.lock_shared();
    if (entryexist(virt) == MMU_PAGE_4K)
    {
        u64 vpn = (virt & 0x0000FFFFFFFFF000);
        u64 va_offset = virt & 0xFFF;
        physaddr phys = this->mmu_tlb_4k.at(vpn);
        result = phys == badphysaddr ? badphysaddr : phys + va_offset;
    }
    else if (entryexist(virt) == MMU_PAGE_2M)
    {
        u64 vpn = (virt & 0x0000FFFFFFE00000);
        u64 va_offset = virt & 0x1FFFFF;
        result = this->mmu_tlb_2m.at(vpn) + va_offset;
    }
    else if (entryexist(virt) == MMU_PAGE_1G)
    {
        u64 vpn = (virt & 0x0000FFFFC0000000);
        u64 va_offset = virt & 0x3FFFFFFF;
        result = this->mmu_tlb_1g.at(vpn) + va_offset;
    }
    else
    {
        result = badphysaddr;
    }
    smtx.unlock_shared();
    return result;
}
void mmu_tlb::entryinsert(uptr virt, uptr phys, mmu_pagetype type)
{
    u64 vpn, pfn;
    smtx.lock();
    switch (type)
    {
    case MMU_PAGE_4K:
        vpn = (virt & 0x0000FFFFFFFFF000);
        if (phys != badphysaddr)
            pfn = (phys & 0x0000FFFFFFFFF000);
        else
            pfn = badphysaddr;
        if (!entryexist(vpn))
            mmu_tlb_4k.insert(pair<u64, u64>(vpn, pfn));
        break;
    case MMU_PAGE_2M:
        vpn = (virt & 0x0000FFFFFFE00000);
        pfn = (phys & 0x0000FFFFFFE00000);
        if (!entryexist(vpn))
            mmu_tlb_2m.insert(pair<u64, u64>(vpn, pfn));
        break;
    case MMU_PAGE_1G:
        vpn = (virt & 0x0000FFFFC0000000);
        pfn = (phys & 0x0000FFFFC0000000);
        if (!entryexist(vpn))
            mmu_tlb_1g.insert(pair<u64, u64>(vpn, pfn));
        break;
    default:
        break;
    }
    smtx.unlock();
}

physaddr mmu_tlb::get_pa_from_dtb_withtlb(uptr va, mmu_pagetype *type)
{
    hw_pte PageEntry[3] = {0};
    hw_pte page;
    va_t vva;
    physaddr phys = 0;
    vva.all = (u64)va;
    if (type)
        *type = MMU_PAGE_INVALID;
    uptr pteaddr = 0, pdeaddr = 0, pdpteaddr = 0;
    pteaddr = get_pte_addr(va);
    if ((phys = virt2phys(pteaddr, MMU_TLB_FORCE_USE)) != badphysaddr)
    {
        goto trans_pte;
    }
    pdeaddr = get_pde_addr(va);
    if ((phys = virt2phys(pdeaddr, MMU_TLB_FORCE_USE)) != badphysaddr)
    {
        goto trans_pde;
    }
    pdpteaddr = get_pdpte_addr(va);
    if ((phys = virt2phys(pdpteaddr, MMU_TLB_FORCE_USE)) != badphysaddr)
    {
        goto trans_pdpte;
    }

    // if not in tlb cache then parse page table

    if (read_phys(dtb + vva.pml4e_index * 8, (u8 *)&page.all, 8) && page.present == 0 || page.large_page == 1)
        return badphysaddr;

    phys = (page.page_frame_number << 12) + vva.pdpte_index * 8;
    entryinsert(pdpteaddr, phys, MMU_PAGE_4K);
trans_pdpte:
    if (read_phys(phys, (u8 *)&page.all, 8) && page.present == 0)
        return badphysaddr;
    if (page.large_page)
    {
        if (type)
            *type = MMU_PAGE_1G;
        u64 off = (u64)va & 0x3FFFFFFF;
        u64 PhysAddr = page.page_frame_number << 12;
        PhysAddr += off;
        return PhysAddr;
    }
    phys = (page.page_frame_number << 12) + vva.pde_index * 8;
    entryinsert(pdeaddr, phys, MMU_PAGE_4K);
trans_pde:
    if (read_phys(phys, (pbyte)&page.all, 8) && page.present == 0)
        return badphysaddr;
    if (page.large_page)
    {
        if (type)
            *type = MMU_PAGE_2M;
        u64 off = (u64)va & 0x1FFFFF;
        u64 PhysAddr = page.page_frame_number << 12;
        PhysAddr += off;
        return PhysAddr;
    }
    phys = (page.page_frame_number << 12) + vva.pte_index * 8;
    entryinsert(pteaddr, phys, MMU_PAGE_4K);
trans_pte:
    if (read_phys(phys, (pbyte)&page.all, 8) && page.present == 0)
        return badphysaddr;
    if (type)
        *type = MMU_PAGE_4K;
    u64 off = (u64)va & 0xFFF;
    u64 PhysAddr = page.page_frame_number << 12;
    PhysAddr += off;
    return PhysAddr;
}
physaddr mmu_tlb::virt2phys(uptr virt, tlb_mode mode)
{
    virt &= 0x0000FFFFFFFFFFFF;
    if (mode == MMU_TLB_NON_USE)
        return get_pa_from_dtb_withtlb(virt, 0);
    if (entryget(virt) == badphysaddr)
    {
        if (mode == MMU_TLB_AUTO_USE)
        {
            mmu_pagetype type = MMU_PAGE_INVALID;
            physaddr phys = get_pa_from_dtb_withtlb(virt, &type);
            if (phys != badphysaddr && type != MMU_PAGE_INVALID)
            {
                entryinsert(virt, phys, type);
                // tlb->mmu_tlb.insert(pair<u64, u64>(vpn, phys));
                return phys;
            }
            else
            {
                entryinsert(virt, badphysaddr, MMU_PAGE_4K);
                return badphysaddr;
            }
        }
        else if (mode == MMU_TLB_FORCE_USE)
            return badphysaddr;
    }
    return entryget(virt);
}

bool mmu_tlb::rw_virt_from_tlb(uptr virt, u8 *pb, u32 cb, bool iswrite)
{
    auto rwphys = 
            [&](physaddr pa, u8* pb, u32 cb)->bool
            {
                if(iswrite)
                {
                    return this->write_phys(pa,pb,cb);
                }
                else
                {
                    return this->read_phys(pa,pb,cb);
                }
            };
    if (cb == 0)
        return false;
    // DbgBreakPoint();
    u64 StartPhysicalAddress = (u64)virt;
    u64 EndPhysicalAddress = StartPhysicalAddress + cb - 1;
    u64 StartPhysicalAddressPage = StartPhysicalAddress & 0xFFFFFFFFFFFFF000;
    u64 EndPhysicalAddressPage = EndPhysicalAddress & 0xFFFFFFFFFFFFF000;
    u64 StartPFN = MmiGetPhysicalPFN(StartPhysicalAddress);
    u64 EndPFN = MmiGetPhysicalPFN(EndPhysicalAddress);
    u32 ThroughPages = EndPFN - StartPFN;
    if (ThroughPages == 0)
    {
        u64 phy = virt2phys((uptr)virt);
        if (phy != badphysaddr)
        {
            if (!rwphys(phy, pb, cb))
                return false;
        }
        else
        {
            memset(pb, 0, cb);
            return false;
        }
    }
    else
    {
        u64 FirstReadPage = StartPhysicalAddressPage;
        u32 FirstReadOffset = StartPhysicalAddress & 0x0000000000000FFF;
        u32 FirstReadSize = PAGE_SIZE - FirstReadOffset;
        u64 phy = virt2phys((uptr)virt);
        if (phy != badphysaddr)
        {
            if (!rwphys(phy, pb, FirstReadSize))
                return false;
        }
        else
        {
            memset(pb, 0, FirstReadSize);
            return false;
        }

        u32 FullPageNum = ThroughPages - 1;
        if (FullPageNum)
        {
            for (u32 i = 0; i < FullPageNum; ++i)
            {
                u64 FullPageAddress = FirstReadPage + PAGE_SIZE + i * PAGE_SIZE;
                pbyte pDst = (pbyte)pb + FirstReadSize + (i * PAGE_SIZE);
                phy = virt2phys((uptr)FullPageAddress);
                if (phy != badphysaddr)
                {
                    if (!rwphys(phy, pDst, PAGE_SIZE))
                        return false;
                }
                else
                {
                    memset(pDst, 0, PAGE_SIZE);
                    return false;
                }
            }
        }

        u64 EndReadPage = EndPhysicalAddressPage;
        u32 EndReadSize = (EndPhysicalAddress & 0x0000000000000FFF) + 1;
        u32 EndReadDstOffset = (ThroughPages - 1) * PAGE_SIZE + FirstReadSize;

        phy = virt2phys((uptr)EndReadPage);
        if (phy != badphysaddr)
        {
            if (!rwphys(phy, ((pbyte)pb) + EndReadDstOffset, EndReadSize))
                return false;
        }
        else
        {
            memset(((pbyte)pb) + EndReadDstOffset, 0, EndReadSize);
            return false;
        }
    }
    return true;
}
bool mmu_tlb::read_virt_from_tlb(uptr virt, u8 *pb, u32 cb)
{
    return rw_virt_from_tlb(virt,pb,cb,false);
}
bool mmu_tlb::write_virt_from_tlb(uptr virt, u8 *pb, u32 cb)
{
    return rw_virt_from_tlb(virt,pb,cb,true);
}

#define rr this->r
#define r(v) (this->r<decltype(v)>(&(v)))
#define parse(pos, _1, _2) ((ULONG64)(rr<LONG>((pos) + (_1)) + (pos) + (_2)))

#include "windows_defs.h"
#include "../include/dma_memsch.h"

u64 mmu::memsch(uptr virt, u64 sz, const char *pattern)
{
    u64 pos = dma_memsch::sd_search(this, virt, sz, pattern);
    return pos == MEMSCH_BADADDR ? 0 : virt + pos;
}
int mmu::memsch(uptr virt, u64 sz, const char *pattern, std::function<bool(u64 pos)> callback)
{
    uptr curr = virt;
    u64 left = sz;
    int patlen = dma_memsch::sd_patlen(pattern);

    int hits = 0;
    for (;;)
    {
        u64 pos = dma_memsch::sd_search(this, curr, left, pattern);
        if (pos == MEMSCH_BADADDR)
            break;
        hits++;
        if (!callback(curr + pos))
            break;

        u64 skiplen = pos + patlen;
        curr += skiplen;
        if (left <= skiplen)
        {
            break;
        }
        left -= skiplen;
    }
    return hits;
}

u64 mmu::scanmem(u64 begin, u64 end, const char *pattern)
{
    bool flag_finded = false;
    u64 result = MEMSCH_BADADDR;

    auto cb = [&](u64 start, u64 sz, u64 rwx) -> bool
    {
        u64 pos = dma_memsch::sd_search(this, start, sz, pattern);
        if (pos != MEMSCH_BADADDR)
        {
            result = pos + start;
            flag_finded = true;
            return false;
        }
        return true;
    };
    enummem(begin, end, cb);
    if (!flag_finded)
        return MEMSCH_BADADDR;
    return result - begin;
}
int mmu::scanmem(u64 begin, u64 end, const char *pattern, std::function<bool(u64 pos)> callback)
{
    int final_result = 0;

    auto cb = [&](u64 start, u64 sz, u64 rwx) -> bool
    {
        uptr curr = start;
        u64 left = sz;
        int patlen = dma_memsch::sd_patlen(pattern);
        bool result = true;
        int hits = 0;
        for (;;)
        {
            u64 pos = dma_memsch::sd_search(this, curr, left, pattern);
            if (pos == MEMSCH_BADADDR)
                break;
            hits++;
            if (!callback(curr + pos))
            {
                result = false;
                break;
            }

            u64 skiplen = pos + patlen;
            curr += skiplen;
            if (left <= skiplen)
            {
                break;
            }
            left -= skiplen;
        }
        final_result += hits;
        return result;
    };

    enummem(begin, end, cb);
    return final_result;
}

u64 mmu::enummem(uptr start_va, uptr end_va, std::function<bool(u64 start, u64 sz, u64 rwx)> callback)
{
    u64 record_start = 0;
    u64 record_sz = 0;
    u64 record_rwx = 0;
    bool recording = false;
    int record_count = 0;

    auto parse_rwx = [&](hw_pte pte) -> u64
    {
        u64 r = 0;
        if (pte.present)
        {
            r |= 1;
        }
        if (pte.allow_write_access)
        {
            r |= 2;
        }
        if (!pte.non_executable)
        {
            r |= 4;
        }
        return r;
    };
    auto record = [&](u64 start_addr, u64 sz, u64 rwx) -> bool
    {
        if ((start_addr + sz) >= end_va)
        {
            record_sz += end_va - start_addr;
            if (!callback(record_start, record_sz, record_rwx))
            {
                record_count++;
                recording = false;
                return false;
            }

            record_count++;
            recording = false;
            return false;
        }
        if (recording)
        {
            if (record_start + record_sz != start_addr || record_rwx != rwx)
            {
                if (!callback(record_start, record_sz, record_rwx))
                {
                    record_count++;
                    recording = false;
                    return false;
                }
                record_count++;
                recording = false;
                goto set_record_val;
            }
            else
            {
                record_sz += sz;
            }
        }
        else
        {
        set_record_val:
            record_start = start_addr;
            record_sz = sz;
            record_rwx = rwx;
            recording = true;
        }
        return true;
    };

    va_t stva;
    va_t edva;
    stva.all = start_va;
    edva.all = end_va;
    const u64 pml4e_sz = ((u64)0x1000 * 512 * 512 * 512);
    const u64 pdpte_sz = (0x1000 * 512 * 512);
    const u64 pde_sz = (0x1000 * 512);
    const u64 pte_sz = (0x1000);

    qword pml4[0x200];
    if (!read_phys(this->get_dtb(), (pbyte)pml4, 0x1000))
        return 0;


    for (int pml4e_idx = stva.pml4e_index; pml4e_idx <= edva.pml4e_index; pml4e_idx++)
    {
        va_t pml4_curva(0, pml4e_idx);
        if (pml4_curva.all >= end_va)
            return record_count;
        hw_pte pml4e;
        qword pdpt[0x200];
        pml4e.all = pml4[pml4e_idx];
        if (!pml4e.present || !read_phys((physaddr)(pml4e.page_frame_number << 12), (pbyte)pdpt, 0x1000))
            continue;

        bool first_pml4 = pml4e_idx == stva.pml4e_index;
        for (int pdpte_idx = first_pml4 ? stva.pdpte_index : 0; pdpte_idx <= 0x1ff; pdpte_idx++)
        {
            hw_pte pdpte;
            pdpte.all = pdpt[pdpte_idx];
            va_t pdpte_curva(0, pml4e_idx, pdpte_idx);
            if (pdpte_curva.all >= end_va)
                return record_count;

            if (!pdpte.present)
                continue;
            // printf("%d\n", pdpte.PAT);
            if (pdpte.large_page)
            {
                va_t va(0, pml4e_idx, pdpte_idx);
                if (!record(va.all, pdpte_sz, parse_rwx(pdpte)))
                    return record_count;
                continue;
            }
            qword pd[0x200];
            if (!read_phys((physaddr)(pdpte.page_frame_number << 12), (pbyte)pd, 0x1000))
                continue;

            bool first_pdpte = first_pml4 && pdpte_idx == stva.pdpte_index;
            for (int pde_idx = first_pdpte ? stva.pde_index : 0; pde_idx <= 0x1ff; pde_idx++)
            {
                hw_pte pde;
                pde.all = pd[pde_idx];
                va_t pde_curva(0, pml4e_idx, pdpte_idx, pde_idx);
                if (pde_curva.all >= end_va)
                    return record_count;
                if (
                    !pde.present || pde.page_frame_number == 0x5000 || pde.page_frame_number == 0x4f01)
                {
                    continue;
                }
                if (pde.large_page)
                {
                    va_t va(0, pml4e_idx, pdpte_idx, pde_idx);
                    if (!record(va.all, pde_sz, parse_rwx(pde)))
                        return record_count;
                    continue;
                }
                qword pt[0x200];
                if (!read_phys((physaddr)(pde.page_frame_number << 12), (pbyte)pt, 0x1000))
                    continue;

                bool first_pde = first_pdpte && pde_idx == stva.pde_index;
                for (int pte_idx = first_pde ? stva.pte_index : 0; pte_idx <= 0x1ff; pte_idx++)
                {
                    hw_pte pte;
                    pte.all = pt[pte_idx];
                    va_t pte_curva(0, pml4e_idx, pdpte_idx, pde_idx, pte_idx);
                    if (pte_curva.all >= end_va)
                        return record_count;
                    if (
                        !pte.present || pte.page_frame_number == 0x200)
                    {
                        continue;
                    }
                    va_t va(0, pml4e_idx, pdpte_idx, pde_idx, pte_idx);
                    if (!record(va.all, pte_sz, parse_rwx(pte)))
                        return record_count;
                }
            }
        }
    }
    if (recording)
    {
        if (!callback(record_start, record_sz, record_rwx))
            return false;
    }
    return record_count;
}
std::list<std::tuple<u64, u64, u64>>
mmu::traversemem(uptr start_va, uptr end_va)
{
    std::list<std::tuple<u64, u64, u64>> r{};
    enummem(start_va, end_va, [&](u64 start, u64 sz, u64 rwx) -> bool
            {
        r.push_back(std::make_tuple(start,sz,rwx));
        return true; });
    return std::move(r);
}
