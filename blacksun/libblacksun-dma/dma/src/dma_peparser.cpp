#include <dma_peparser.h>
#include <dma_mmu.h>
#include <dma_type.h>
#include <memory>

#include <string>
#include <vector>

#include "windows_defs.h"

#define fetch_base_data                                                        \
  IMAGE_DOS_HEADER idh;                                                        \
  IMAGE_NT_HEADERS64 inh;                                                      \
  std::vector<IMAGE_SECTION_HEADER> ish;                                       \
  idh = r<decltype(idh)>(base);                                                \
  inh = r<decltype(inh)>(base + idh.e_lfanew);                                 \
  u64 pish = ((u64)(base + idh.e_lfanew) +                                     \
              offsetof(IMAGE_NT_HEADERS, OptionalHeader) +                     \
              inh.FileHeader.SizeOfOptionalHeader);                            \
  for (int i = 0; i < inh.FileHeader.NumberOfSections; i++) {                  \
    ish.push_back(                                                             \
        r<IMAGE_SECTION_HEADER>(pish + sizeof(IMAGE_SECTION_HEADER) * i));     \
  }

bool peMemory64::isvalid() {
  fetch_base_data;
  return idh.e_magic == 'ZM' && inh.Signature == 'EP';
}
u64 peMemory64::sectaddr(const std::string &sectname) {
  fetch_base_data;
  for (auto &&sect : ish) {
    char Name[9];
    memset(Name, 0, 9);
    *(u64 *)Name = *(u64 *)sect.Name;
    std::string strName(Name);
    if (strName == sectname) {
      DWORD virtaddr = sect.VirtualAddress;
      return virtaddr + base;
    }
  }
  return 0;
}
u64 peMemory64::sectsize(const std::string &sectname) {
  fetch_base_data;
  for (auto &&sect : ish) {
    char Name[9];
    memset(Name, 0, 9);
    *(u64 *)Name = *(u64 *)sect.Name;
    std::string strName(Name);
    if (strName == sectname) {
      return sect.Misc.VirtualSize;
    }
  }
  return 0;
}
u64 peMemory64::scansect(const std::string &sectname,
                         const std::string &pattern) {
  fetch_base_data;
  u64 addr = sectaddr(sectname);
  u64 size = sectsize(sectname);
  if (addr != 0 && size != 0) {
    return memsch((u64)base + addr, size, pattern.c_str());
  }
  return 0;
}
u64 peMemory64::funcbegin(u64 pc) {
  fetch_base_data;
  DWORD va = inh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION]
                 .VirtualAddress;
  DWORD sz =
      inh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size;
  if (va) {
    for (int i = 0; i < sz / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY); i++) {
      IMAGE_RUNTIME_FUNCTION_ENTRY func =
          r<decltype(func)>(base + va + i * sizeof(decltype(func)));
      ULONG64 Begin = func.BeginAddress + (ULONG64)base;
      ULONG64 End = func.EndAddress + (ULONG64)base;
      if (pc >= Begin && pc < End) {
        return (u64)Begin;
      }
    }
  }
  return 0;
}
u64 peMemory64::funcend(u64 pc) {
  fetch_base_data;
  DWORD va = inh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION]
                 .VirtualAddress;
  DWORD sz =
      inh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size;
  if (va) {
    for (int i = 0; i < sz / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY); i++) {
      IMAGE_RUNTIME_FUNCTION_ENTRY func =
          r<decltype(func)>(base + va + i * sizeof(decltype(func)));
      ULONG64 Begin = func.BeginAddress + (ULONG64)base;
      ULONG64 End = func.EndAddress + (ULONG64)base;
      if (pc >= Begin && pc < End) {
        return (u64)End;
      }
    }
  }
  return 0;
}
std::tuple<std::string, std::string, uint32_t> peMemory64::pdbinfo() {
  fetch_base_data;
  DWORD rva = inh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG]
                  .VirtualAddress;
  DWORD size =
      inh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
  if (rva) {
    uint32_t through_size = 0;
    while (through_size < size) {
      IMAGE_DEBUG_DIRECTORY dbg = r<decltype(dbg)>(base + rva + through_size);
      if (dbg.Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
        struct GUID {
          uint32_t Data1;
          uint16_t Data2;
          uint16_t Data3;
          uint8_t Data4[8];
        };
        struct raw_debug_info {
          DWORD signature;
          GUID guid;
          DWORD age;
          char pdb_file_name[1];
        };
        raw_debug_info raw = r<decltype(raw)>(dbg.AddressOfRawData + base);
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::uppercase;
        ss << std::setw(8) << raw.guid.Data1;
        ss << std::setw(4) << raw.guid.Data2;
        ss << std::setw(4) << raw.guid.Data3;
        for (unsigned char i : raw.guid.Data4) {
          ss << std::setw(2) << static_cast<std::uint32_t>(i);
        }
        raw_debug_info *praw = (decltype(praw))(dbg.AddressOfRawData + base);
        char buffer[1024] = {};
        if (read_string((u64)(&((praw)->pdb_file_name)), buffer, 1024)) {
          return std::make_tuple<std::string, std::string, uint32_t>(
              std::string(buffer), ss.str(), std::move(raw.age));
        }
      }
      through_size += sizeof(decltype(dbg));
    }
  }
  return std::make_tuple<std::string, std::string, uint32_t>("", "", 0);
}
