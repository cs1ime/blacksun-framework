#pragma once
#ifndef _DMA_MMU_H_
#define _DMA_MMU_H_

#include <dma_type.h>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <utility>
#include <memory>

class physmem_accessor {
public:
  virtual bool read_physical_memory(physaddr pa, u8 *pb, size_t cb) = 0;
  virtual bool write_physical_memory(physaddr pa, u8 *pb, size_t cb) = 0;

private:
};

enum tlb_mode { MMU_TLB_AUTO_USE = 0, MMU_TLB_NON_USE, MMU_TLB_FORCE_USE };
enum mmu_pagetype {
  MMU_PAGE_INVALID = 0,
  MMU_PAGE_4K,
  MMU_PAGE_2M,
  MMU_PAGE_1G
};

class mmu_initializer {
public:
  mmu_initializer(physaddr _dtb, u64 _pte_base,
                  std::shared_ptr<physmem_accessor> accessor)
      : m_accessor(accessor) {
    dtb = _dtb & 0x000FFFFFFFFFF000;
    ptebase = _pte_base;
    pdebase = get_pte_addr((u64)ptebase);
    pdptebase = get_pte_addr((u64)pdebase);
    pml4ebase = get_pte_addr((u64)pdptebase);
  }
  mmu_initializer(const mmu_initializer &_m) {
    this->dtb = _m.dtb;
    this->ptebase = _m.ptebase;
    this->pdebase = _m.pdebase;
    this->pdptebase = _m.pdptebase;
    this->pml4ebase = _m.pml4ebase;
    this->m_accessor = _m.m_accessor;
  }
  mmu_initializer(const mmu_initializer &_m, physaddr _dtb) {
    this->dtb = _dtb & 0x000FFFFFFFFFF000;
    this->ptebase = _m.ptebase;
    this->pdebase = _m.pdebase;
    this->pdptebase = _m.pdptebase;
    this->pml4ebase = _m.pml4ebase;
    this->m_accessor = _m.m_accessor;
  }
  mmu_initializer &operator=(const mmu_initializer &_m) {
    this->dtb = _m.dtb;
    this->ptebase = _m.ptebase;
    this->pdebase = _m.pdebase;
    this->pdptebase = _m.pdptebase;
    this->pml4ebase = _m.pml4ebase;
    this->m_accessor = _m.m_accessor;
    return *this;
  }

  bool read_physical_memory(physaddr pa, u8 *pb, size_t cb) {
    return this->m_accessor->read_physical_memory(pa, pb, cb);
  }
  bool write_physical_memory(physaddr pa, u8 *pb, size_t cb) {
    return this->m_accessor->write_physical_memory(pa, pb, cb);
  }

  u64 get_pte_base() { return ptebase; }
  u64 get_pde_base() { return pdebase; }
  u64 get_pdpte_base() { return pdptebase; }
  u64 get_pml4e_base() { return pml4ebase; }

  physaddr get_dtb() { return dtb; }

  void set_pte_base(u64 _pte_base) {
    ptebase = _pte_base;
    pdebase = get_pte_addr((u64)ptebase);
    pdptebase = get_pte_addr((u64)pdebase);
    pml4ebase = get_pte_addr((u64)pdptebase);
  }
  void set_dtb(physaddr _dtb) { dtb = _dtb; }

  u64 get_pte_addr(u64 addr) {
    return ((((((u64)addr) & 0x0000FFFFFFFFF000) >> 12) << 3) + ptebase);
  }
  u64 get_pde_addr(u64 addr) {
    return ((((((u64)addr) & 0x0000FFFFFFFFF000) >> 21) << 3) + pdebase);
  }
  u64 get_pdpte_addr(u64 addr) {
    return ((((((u64)addr) & 0x0000FFFFFFFFF000) >> 30) << 3) + pdptebase);
  }
  u64 get_pml4e_addr(u64 addr) {
    return ((((((u64)addr) & 0x0000FFFFFFFFF000) >> 39) << 3) + pml4ebase);
  }

#define MmiGetPhysicalPFN(p) (((u64)(p)&0xFFFFFFFFFFFFF000) >> 12)

  bool read_physical_memory_with_fault(physaddr pa, u8 *pb, u32 cb) {
    u64 pfn = pa & 0xFFFFFFFFFFFFF000;
    faultaddr_smtx.lock_shared();
    if (faultaddr.find(pfn) != faultaddr.end()) {
      faultaddr_smtx.unlock_shared();
      return false;
    }
    faultaddr_smtx.unlock_shared();

    if (!read_physical_memory(pa, pb, cb)) {
      faultaddr_smtx.lock();
      faultaddr.insert(std::pair<u64, bool>(pfn, true));
      faultaddr_smtx.unlock();
      return false;
    }
    return true;
  }
  bool read_phys(physaddr pa, u8 *pb, u32 cb) {
    if (pa == D_BADPHYSADDR)
      return false;
    if (cb == 0)
      return true;
    u64 StartPhysicalAddress = (u64)pa;
    u64 EndPhysicalAddress = StartPhysicalAddress + cb - 1;
    u64 StartPhysicalAddressPage = StartPhysicalAddress & 0xFFFFFFFFFFFFF000;
    u64 EndPhysicalAddressPage = EndPhysicalAddress & 0xFFFFFFFFFFFFF000;
    u64 StartPFN = MmiGetPhysicalPFN(StartPhysicalAddress);
    u64 EndPFN = MmiGetPhysicalPFN(EndPhysicalAddress);
    u32 ThroughPages = EndPFN - StartPFN;
    if (ThroughPages == 0) {
      u64 phy = pa;
      // memcpy(Dst, ((PUCHAR)Pending) + offset, Lenth);
      read_physical_memory_with_fault(phy, pb, cb);
    } else {
      u64 FirstReadPage = StartPhysicalAddressPage;
      u32 FirstReadOffset = StartPhysicalAddress & 0x0000000000000FFF;
      u32 FirstReadSize = D_PAGE_SIZE - FirstReadOffset;
      u64 phy = pa;

      read_physical_memory_with_fault(phy, pb, FirstReadSize);

      u32 FullPageNum = ThroughPages - 1;
      if (FullPageNum) {
        for (u32 i = 0; i < FullPageNum; ++i) {
          u64 FullPageAddress = FirstReadPage + D_PAGE_SIZE + i * D_PAGE_SIZE;
          u8 *pDst = (u8 *)pb + FirstReadSize + (i * D_PAGE_SIZE);
          phy = FullPageAddress;
          if (phy == D_BADPHYSADDR)
            return false;
          read_physical_memory_with_fault(phy, pDst, D_PAGE_SIZE);
        }
      }
      u64 EndReadPage = EndPhysicalAddressPage;
      u32 EndReadSize = (EndPhysicalAddress & 0x0000000000000FFF) + 1;
      u32 EndReadDstOffset = (ThroughPages - 1) * D_PAGE_SIZE + FirstReadSize;

      phy = EndPhysicalAddressPage;
      if (phy == D_BADPHYSADDR)
        return false;

      read_physical_memory_with_fault(phy, pb + EndReadDstOffset, EndReadSize);
    }
    return true;
  }

  bool write_phys(physaddr pa, u8 *pb, u32 cb) {
    if (pa == D_BADPHYSADDR)
      return false;
    return write_physical_memory(pa, pb, cb);
  }

private:
  std::shared_mutex faultaddr_smtx;
  std::unordered_map<u64, bool> faultaddr;
  std::shared_ptr<physmem_accessor> m_accessor;

protected:
  physaddr dtb = 0;
  u64 ptebase = 0;
  u64 pdebase = 0;
  u64 pdptebase = 0;
  u64 pml4ebase = 0;
};

class mmu_tlb : public mmu_initializer {
public:
  explicit mmu_tlb(const mmu_initializer &_m);
  void invalidate();
  physaddr entryget(uptr virt);
  void entryinsert(uptr virt, uptr phys, mmu_pagetype type);
  physaddr virt2phys(uptr virt, tlb_mode mode = MMU_TLB_AUTO_USE);
  bool read_virt_from_tlb(uptr virt, u8 *pb, u32 cb);
  bool write_virt_from_tlb(uptr virt, u8 *pb, u32 cb);
  bool chkvirt(u64 virt) { return virt2phys(virt) != D_BADPHYSADDR; }
  bool chkvirt(ptr virt) { return chkvirt((u64)virt); }
  bool read_virt(u64 virt, ptr pb, u32 cb) {
    return read_virt_from_tlb(virt, (u8 *)pb, cb);
  };
  bool write_virt(u64 virt, ptr pb, u32 cb) {
    return write_virt_from_tlb(virt, (u8 *)pb, cb);
  }
  template <typename T> T r(u64 virt) {
    T result;
    memset(&result, 0, sizeof(T));
    if (read_virt(virt, &result, sizeof(T))) {
      return result;
    }
    return result;
  };
  template <typename T> T r(ptr virt) { return r<T>((u64)virt); };
  template <typename T> bool w(u64 virt, T data) {
    return write_virt(virt, &data, sizeof(T));
  };
  template <typename T> bool w(ptr virt, T data) {
    return w<T>((u64)virt, data);
  }

  void invtlb() { invalidate(); }

private:
  bool rw_virt_from_tlb(uptr virt, u8 *pb, u32 cb, bool iswrite);
  mmu_pagetype entryexist(uptr virt);
  std::shared_mutex smtx;
  std::unordered_map<u64, u64> mmu_tlb_4k;
  std::unordered_map<u64, u64> mmu_tlb_2m;
  std::unordered_map<u64, u64> mmu_tlb_1g;

  physaddr get_pa_from_dtb_withtlb(uptr va, mmu_pagetype *type);
};
class mmu_basic : public mmu_tlb {
protected:
public:
  explicit mmu_basic(const mmu_initializer &_m) : mmu_tlb(_m) {}
  ~mmu_basic() {}
};
class mmu_cache {
private:
  u8 cache1[0x1000];
  u64 prev_page1 = 0;
  bool is_using1 = false;
  bool is_fault1 = false;

public:
  explicit mmu_cache() {
    prev_page1 = 0;
    is_using1 = 0;
    is_fault1 = 0;
  }
  u8 *cache() { return cache1; }
  u64 prev_page() { return prev_page1; }
  bool is_using() { return is_using1; }
  bool is_fault() { return is_fault1; }
  void set_prev_page(u64 prev_page) { prev_page1 = prev_page; }
  void set_using() { is_using1 = true; }
  void set_fault() { is_fault1 = true; }
  void clear_using() { is_using1 = false; }
  void clear_fault() { is_fault1 = false; }

  void invalidate() {
    prev_page1 = 0;
    is_using1 = 0;
    is_fault1 = 0;
  }
};
class mmu_fast : public mmu_basic {
private:
  bool read_from_cache(u8 *pcache, u64 virt, ptr pb, u32 cb) {
    u64 page = virt & 0xFFFFFFFFFFFFF000;
    u64 pageoffset = virt & 0xFFF;
    u64 pagend = (virt + cb - 1) & 0xFFFFFFFFFFFFF000;
    if (page == pagend) {
      memcpy(pb, pcache + pageoffset, cb);
      return true;
    } else {
      // printf("addr:%llX\n", page);
      u64 edge_sz = 0x1000 - pageoffset;
      memcpy(pb, pcache + pageoffset, edge_sz);
      u64 next_page = page + 0x1000;
      u64 overflow_sz = cb - edge_sz;
      return this->read_virt(next_page, (u8 *)pb + edge_sz, overflow_sz);
    }
  }

public:
  explicit mmu_fast(const mmu_initializer &_m) : mmu_basic(_m) { ; }
  ~mmu_fast() { ; }
  bool read_with_cache(mmu_cache *c, u64 virt, ptr pb, u32 cb) {
    if (cb == 0)
      return true;
    u64 page = virt & 0xFFFFFFFFFFFFF000;
    if (c->is_using() && page == c->prev_page()) {
      if (c->is_fault())
        return false;
      return read_from_cache(c->cache(), virt, pb, cb);
    } else {
      c->set_prev_page(page);
      c->set_using();
      // printf("page:%p\n", page);
      if (this->read_virt(page, c->cache(), 0x1000)) {
        c->clear_fault();
        return read_from_cache(c->cache(), virt, pb, cb);
      } else {
        c->set_fault();
        return false;
      }
    }
  }
  int read_string(u64 virt, pstr pb, u32 cb) {
    mmu_cache c;
    int length = 0;
    for (int i = 0; i < cb; i++) {
      u8 v = 0;
      if (read_with_cache(&c, virt + i, &v, 1)) {
        if (v != 0) {
          length++;
          *((u8 *)pb + i) = v;
        }
      }
    }
    return length;
  }
  int read_wstring(u64 virt, pwstr pb, u32 cb) {
    mmu_cache c;
    int length = 0;
    for (int i = 0; i < cb; i++) {
      wchar_t v = 0;
      if (read_with_cache(&c, virt + i * 2, &v, 2)) {
        // printf("%c\n",v);
        if (v != 0) {
          length++;
          pb[i] = v;
        }
      }
    }
    return length;
  }
};

class mmu : public mmu_fast {
private:
public:
  explicit mmu(const mmu_initializer &_m) : mmu_fast(_m) { ; }
  ~mmu() { ; }

  u64 memsch(uptr virt, u64 sz, const char *pattern);
  int memsch(uptr virt, u64 sz, const char *pattern,
             std::function<bool(u64 pos)> callback);
  u64 scanmem(u64 begin, u64 end, const char *pattern);
  int scanmem(u64 begin, u64 end, const char *pattern,
              std::function<bool(u64 pos)> callback);

  u64 enummem(uptr start_va, uptr end_va,
              std::function<bool(u64 start, u64 sz, u64 rwx)> callback);

  // this pair contains memory_regin_start_va memory_regin_size
  std::list<std::tuple<u64, u64, u64>> traversemem(uptr start_va, uptr end_va);
};

#endif // !_DMA_MMU_H_
