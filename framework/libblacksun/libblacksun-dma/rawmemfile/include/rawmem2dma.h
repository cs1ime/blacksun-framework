#pragma once
#ifndef _LEECH_TO_DMA_H_
#define _LEECH_TO_DMA_H_

#include <dma.h>
#include <string>

class rawmem2dma : public physmem_accessor {
public:
  explicit rawmem2dma(std::string filename);
  ~rawmem2dma();
  bool valid() { return m_inited; }
  bool read_physical_memory(physaddr pa, u8 *pb, size_t cb) override;
  bool write_physical_memory(physaddr pa, u8 *pb, size_t cb) override;

private:
  bool m_inited = false;
  int m_rawmemfd = 0;
  void *m_mapped_addr = nullptr;
  long m_mapped_size = 0;
};

#endif // !_LEECH_TO_DMA_H_
