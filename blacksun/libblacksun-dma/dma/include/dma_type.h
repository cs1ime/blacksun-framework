#pragma once
#ifndef _DMA_INTERFACE_TYPE_H_
#define _DMA_INTERFACE_TYPE_H_

#include <stdint.h>

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef u64 uptr;
typedef u64 physaddr;

typedef void* ptr;

typedef char* pstr;
typedef wchar_t * pwstr;

#define D_PAGE_SIZE (0x1000)
#define D_BADPHYSADDR ((physaddr)-1)




#endif // !_DMA_INTERFACE_TYPE_H_


