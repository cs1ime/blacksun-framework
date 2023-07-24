//
// Created by RSP on 2023/3/27.
//

#ifndef LIBDMAWRAPPER_SRC_OLDDEFINE_H
#define LIBDMAWRAPPER_SRC_OLDDEFINE_H

#define badphysaddr D_BADPHYSADDR
#define PAGE_SIZE D_PAGE_SIZE

typedef u8 byte, *pbyte;
typedef u16 word, *pword;
typedef u32 dword, *pdword;
typedef u64 qword, *pqword;

union hw_pte
{
    u64 all;
    struct
    {
        u64 present : 1;
        u64 allow_write_access : 1;
        u64 allow_usermode_access : 1;
        u64 page_write_through : 1;
        u64 page_cache_disable : 1;
        u64 accessed : 1;
        u64 dirty : 1;      // only for large page, indicates whether software has written
        u64 large_page : 1; // in pml4e,this bit must be 0
        u64 global : 1;
        u64 copy_on_write : 1;
        u64 ignored1 : 1;
        u64 PAT : 1; // only for large page
        u64 page_frame_number : 40;
        u64 ignored2 : 7;
        u64 protection_key : 4;
        u64 non_executable : 1;
    };
};
union va_t
{

    u64 all;
    struct
    {
        u64 offset : 12;
        u64 pte_index : 9;
        u64 pde_index : 9;
        u64 pdpte_index : 9;
        u64 pml4e_index : 9;
        u64 head : 16;
    };
    va_t(word head = 0, int pml4e_idx = 0, int pdpte_idx = 0, int pde_idx = 0, int pte_idx = 0, int offset = 0)
    {
        this->head = head;
        this->pml4e_index = pml4e_idx;
        this->pdpte_index = pdpte_idx;
        this->pde_index = pde_idx;
        this->pte_index = pte_idx;
        this->offset = offset;
    }
    va_t(u64 virt)
    {
        this->all = virt;
    }
    va_t(int virt)
    {
        this->all = virt;
    }
};

#endif // LIBDMAWRAPPER_SRC_OLDDEFINE_H
