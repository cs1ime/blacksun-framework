#pragma once
#ifndef _DMA_MMUSCH_H_
#define _DMA_MMUSCH_H_

#include "dma_mmu.h"
#define MEMSCH_BADADDR (static_cast<u64>(-1))
namespace dma_memsch {
	u64 sd_search(mmu_fast* fast, uptr buf, u64 buf_len, const char* pattern);
    int sd_patlen(const char* pattern);
};

#endif // !_DMA_MMUSCH_H_


