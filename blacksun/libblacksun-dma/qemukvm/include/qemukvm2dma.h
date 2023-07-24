#ifndef __QEMU_KVM_RO_DMA_H_
#define __QEMU_KVM_RO_DMA_H_

#include <dma.h>
#include <memory>
#include <string>

class qemukvm2dma_impl;

class qemukvm2dma : public physmem_accessor {
public:
  explicit qemukvm2dma(std::string vm);
  bool read_physical_memory(physaddr pa, u8 *pb, size_t cb) override;
  bool write_physical_memory(physaddr pa, u8 *pb, size_t cb) override;
  bool valid();

private:
  std::shared_ptr<qemukvm2dma_impl> impl;
};

#endif