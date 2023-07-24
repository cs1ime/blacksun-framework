#ifndef _DMA_PEPARSER_H_
#define _DMA_PEPARSER_H_

#include <dma_mmu.h>
#include <dma_type.h>

class peMemory64 : public mmu {

public:
  peMemory64(const mmu_initializer &_m, u64 va) : mmu(_m) { base = va; }
  bool isvalid();
  u64 sectaddr(const std::string &sectname);
  u64 sectsize(const std::string &sectname);
  u64 scansect(const std::string &sectname, const std::string &pattern);
  u64 funcbegin(u64 pc);
  u64 funcend(u64 pc);
  std::tuple<std::string, std::string, uint32_t> pdbinfo();

private:
  u64 base;
};

#endif