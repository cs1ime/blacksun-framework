
#ifndef _DMA_INTERFACE_H_
#define _DMA_INTERFACE_H_

#include <dma_mmu.h>
#include <dma_ntutil.h>
#include <dma_symbol.h>
#include <dma_type.h>
#include <memory>

class ntfunc_creator {
public:
  ntfunc_creator(std::shared_ptr<dma_symbol_factory> symbol_factory,
                 std::shared_ptr<physmem_accessor> accessor)
      : m_symbol_factory(symbol_factory), m_accessor(accessor){};
  std::shared_ptr<ntfuncs> try_create();

private:
  physaddr mmu_get_pa_from_dtb(physaddr dtb, uptr va);
  uptr dma_find_MmPfnDataBase_from_dtb(u8 *pb);
  uptr dma_find_selfmapping_ptebase(u8 *pb, physaddr pa);
  bool dma_check_root_tb(u8 *pb, physaddr pa);
  physaddr dma_find_root_tb();
  u64 dma_find_ntoskrnl(physaddr dtb);
  bool read_physical_memory(physaddr pa, u8 *pb, size_t cb) {
    return this->m_accessor->read_physical_memory(pa, pb, cb);
  }
  bool write_physical_memory(physaddr pa, u8 *pb, size_t cb) {
    return this->m_accessor->write_physical_memory(pa, pb, cb);
  }

  std::shared_ptr<mmu> g_sysmmu = nullptr;
  std::shared_ptr<physmem_accessor> m_accessor;
  physaddr g_root_tb = 0;
  u64 g_ptebase = 0;
  u64 g_pfnbase = 0;
  std::shared_ptr<dma_symbol_factory> m_symbol_factory;
};
#endif // !_DMA_INTERFACE_H_
