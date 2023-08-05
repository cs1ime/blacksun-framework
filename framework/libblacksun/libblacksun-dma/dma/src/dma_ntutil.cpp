#include "../include/dma_ntutil.h"
#include "../include/dma_type.h"
#include "../include/dma_winver.h"

#include <codecvt>
#include <functional>
#include <list>
#include <locale>
#include <memory>
#include <tuple>

#include "oldnames_inline.h"

#include "src_olddefine.h"

#define xs

using std::list;
using std::string;

#define p1x(v1) printf(xs("" #v1 "=%08llX\n"), (uint64_t)v1)
#define p1d(v1) printf(xs("" #v1 "=%08lld\n"), (uint64_t)v1)

#define rr m_mmu->r
#define r(v) (m_mmu->r<decltype(v)>(&(v)))
#define parse(pos, _1, _2) ((ULONG64)(rr<LONG>((pos) + (_1)) + (pos) + (_2)))

#define GetBitFieldValue(v, a, b)                                              \
  (((uint64_t)v) << ((sizeof(uint64_t) * 8) - (b + 1)) >>                      \
   ((sizeof(uint64_t) * 8) - (b + 1)) >> (a))

#include "windows_defs.h"

void AnsiToUnicode(LPCSTR AnsiStr, LPWSTR UnicodeStrBuffer, ULONG MaxLenth) {
  int len = strlen(AnsiStr);
  if (len > MaxLenth)
    len = MaxLenth;
  UnicodeStrBuffer[len] = 0;
  for (int i = 0; i < len; ++i) {
    UnicodeStrBuffer[i] = AnsiStr[i];
  }
  return;
}
LPCWSTR GetFileName(LPCWSTR a) {
  LPCWSTR resu = a;

  int len = wcslen(a);
  for (int i = 0; i < len; i++) {
    if (a[i] == L'\\' || a[i] == L'/') {
      resu = &a[i + 1];
    }
  }

  return resu;
}

ULONG64 FindPspCidTable(ntfuncs *funcs) {
  mmu *m_mmu = funcs->getmmu();
  ULONG BuildNumber = funcs->GetBuildNumber();
  u64 ntos = funcs->GetNtoskrnl();
  if (BuildNumber < WIN10) {
    // win7
    // 48 8B D1 48 8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 45
    // 00 00 ?? ?? ?? 48 8B D1 48 8B 0D ?? ?? ?? ?? E8
    ULONG64 pos =
        funcs->kscansect(ntos, xs("PAGE"),
                         xs("48 8B D1 48 8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 45"));

    if (pos) {
      return parse(pos, 6, 10);
    } else {
      pos = funcs->kscansect(
          ntos, xs("PAGE"),
          xs("00 00 ?? ?? ?? 48 8B D1 48 8B 0D ?? ?? ?? ?? E8"));
      if (pos) {
        return parse(pos, 11, 15);
      }
    }
  } else if (BuildNumber >= WIN10 && BuildNumber <= WIN10_1607) {
    // win10 1503-win10 1607
    // 48 8B 05 ?? ?? ?? ?? ?? ?? ?? F7 C1
    // E8 02 00 00 48 8B 0D ?? ?? ?? ?? E8
    // 30 06 00 00 48 8B 0D ?? ?? ?? ?? E8
    ULONG64 pos = funcs->kscansect(
        ntos, xs("PAGE"), xs("E8 02 00 00 48 8B 0D ? ? ? ? ? ? ? ? E8"));

    if (pos) {
      return parse(pos, 7, 11);
    } else {
      pos = funcs->kscansect(ntos, xs("PAGE"),
                             xs("30 06 00 00 48 8B 0D ?? ?? ?? ?? E8"));
      if (pos) {
        return parse(pos, 7, 11);
      } else {
        pos = funcs->kscansect(ntos, xs("PAGE"),
                               xs("48 8B 05 ?? ?? ?? ?? ?? ?? ?? F7 C1"));
        if (pos) {
          return parse(pos, 3, 7);
        }
      }
    }
  } else if (BuildNumber >= WIN10_1703) {
    // win10 1903 - win10 2004
    // 48 8B 05 ?? ?? ?? ?? F7 C1 ?? ?? ?? ?? 74
    // E8 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? ?? ?? ?? ?? ?? 45 33 C9 45 33 C0 ??
    // ?? ?? E8 00 00 48 8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 85 C0
    ULONG64 pos =
        funcs->kscansect(ntos, xs("PAGE"),
                         xs("E8 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? ?? ?? ?? ?? "
                            "?? 45 33 C9 45 33 C0 ?? ?? ?? E8"));
    if (pos) {

      return parse(pos, 8, 12);
    } else {
      pos = funcs->kscansect(
          ntos, xs("PAGE"),
          xs("00 00 48 8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 85 C0"));
      if (pos) {
        return parse(pos, 5, 9);
      }
    }
  }
  return 0;
}
u64 FindObHeaderCookie(ntfuncs *thiz) {
  u64 ObReferenceObjectByPointer =
      thiz->ksym(thiz->GetNtoskrnl(), xs("ObReferenceObjectByPointer"));
  // 0F B6 05 ?? ?? ?? ?? 48 33 C8 48 8D 05 ?? ?? ?? ??
  u64 pos =
      thiz->kscanmem(ObReferenceObjectByPointer, 0x1000,
                     xs("0F B6 05 ?? ?? ?? ?? 48 33 C8 48 8D 05 ?? ?? ?? ??"));
  if (pos == 0)
    return 0;
  mmu *m_mmu = thiz->getmmu();

  return parse(pos, 3, 7);
}

bool ntfuncs::initialize_without_symbol() {
  u64 ntos = ntoskrnl;
  BuildNumber = rr<ULONG>(ksym(ntos, xs("NtBuildNumber"))) & 0xFFFF;
  ;

  PsLoadedModuleList = ksym(ntos, xs("PsLoadedModuleList"));
  if (!PsLoadedModuleList)
    return false;
  PspCidTable = FindPspCidTable(this);
  if (!PspCidTable)
    return false;
  ObHeaderCookie = FindObHeaderCookie(this);
  if (!ObHeaderCookie)
    return false;
  PsProcessType = ksym(ntos, xs("PsProcessType"));
  if (!PsProcessType)
    return false;
  offset_SectionBaseAddress =
      rr<u32>(ksym(ntos, xs("PsGetProcessSectionBaseAddress")) + 3);
  if (!offset_SectionBaseAddress)
    return false;
  offset_Peb = rr<u32>(ksym(ntos, xs("PsGetProcessPeb")) + 3);
  if (!offset_Peb)
    return false;
  offset_WoW64Process = rr<u32>(ksym(ntos, xs("PsGetProcessWow64Process")) + 3);
  if (!offset_WoW64Process)
    return false;
  offset_DirectoryTableBase = 0x28;
  if (!offset_DirectoryTableBase)
    return false;
  offset_ImageFileName =
      rr<u32>(ksym(ntos, xs("PsGetProcessImageFileName")) + 3);
  if (!offset_ImageFileName)
    return false;
  offset_ProcessExitTime =
      rr<u32>(kscanmem(ksym(ntos, xs("PsGetProcessExitTime")), 0x1000,
                       xs("48 8B 80 ?? ?? ?? ?? C3")) +
              3);
  if (!offset_ProcessExitTime)
    return false;

  return true;
}
bool ntfuncs::initialize_with_symbol() {
  auto [name, guid, age] = peMemory64(*m_mmu, ntoskrnl).pdbinfo();
  std::cout << "name: " << name << " guid: " << guid << " age: " << age
            << std::endl;
  auto interface = m_symbol_factory->create_interface(name, guid, age);
  if (!interface)
    return false;

  auto all_structures = interface->get_all_structures();
  auto all_symbols = interface->get_all_symbols();
  m_allpdbs = std::make_shared<std::map<std::string, pdbinfo>>();
  m_allpdbs->insert({"ntoskrnl.exe", {interface, all_structures, all_symbols}});

  try {
    u64 ntos = ntoskrnl;
    BuildNumber = rr<ULONG>(ntoskrnl + all_symbols["NtBuildNumber"]) & 0xFFFF;
    ;

    PsLoadedModuleList = ntoskrnl + all_symbols["PsLoadedModuleList"];
    PspCidTable = ntoskrnl + all_symbols["PspCidTable"];
    ObHeaderCookie = ntoskrnl + all_symbols["ObHeaderCookie"];
    PsProcessType = ntoskrnl + all_symbols["PsProcessType"];
    offset_SectionBaseAddress =
        all_structures["_EPROCESS"]["SectionBaseAddress"].offset;
    offset_Peb = all_structures["_EPROCESS"]["Peb"].offset;
    offset_WoW64Process = all_structures["_EPROCESS"]["WoW64Process"].offset;
    offset_DirectoryTableBase =
        all_structures["_KPROCESS"]["DirectoryTableBase"].offset;
    offset_ImageFileName = all_structures["_EPROCESS"]["ImageFileName"].offset;
    offset_ProcessExitTime = all_structures["_EPROCESS"]["ExitTime"].offset;
  } catch (...) {
    std::cerr << "file: " << __FILE__ << " line:" << __LINE__ << '\n';
    return false;
  }

  return true;
}

std::shared_ptr<std::map<std::string, pdbinfo>> ntfuncs::allpdbs() {
  return m_allpdbs;
}

bool ntfuncs::is_valid() { return m_valid; }
ntfuncs::ntfuncs(const mmu_initializer &_m, u64 ntos,
                 std::shared_ptr<dma_symbol_factory> symbol_factory)
    : m_valid(false), m_symbol_factory(symbol_factory) {
  m_mmu = new mmu(_m);
  this->ntoskrnl = ntos | 0xFFFF000000000000;
  if (m_symbol_factory != nullptr) {
    m_valid = initialize_with_symbol();
  } else {
    m_valid = initialize_without_symbol();
  }
}
ntfuncs::~ntfuncs() {
  if (m_mmu) {
    delete m_mmu;
  }
}

u64 ntfuncs::ksym(uptr mod, const char *symname) {
  IMAGE_DOS_HEADER *idh = (IMAGE_DOS_HEADER *)mod;
  IMAGE_NT_HEADERS64 *inh = (IMAGE_NT_HEADERS64 *)(r(idh->e_lfanew) + (u64)idh);
  IMAGE_EXPORT_DIRECTORY *ied =
      (IMAGE_EXPORT_DIRECTORY
           *)((u64)mod +
              r(inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
                    .VirtualAddress));

  DWORD NumberOfNames = r(ied->NumberOfNames);
  for (int i = 0; i < NumberOfNames; i++) {
    USHORT index =
        rr<USHORT>(&((USHORT *)((u64)mod + r(ied->AddressOfNameOrdinals)))[i]);
    ULONG NameRVA =
        rr<ULONG>(&((ULONG *)((u64)mod + r(ied->AddressOfNames)))[i]);
    PCSTR Name = (PCSTR)(((ULONG64)mod) + NameRVA);
    CHAR cName[40] = {};
    memset(cName, 0, 40);
    m_mmu->read_string((u64)Name, cName, 40);

    if (!strcmp(cName, symname)) {
      ULONG FunRVA =
          rr<ULONG>(&((ULONG *)((u64)mod + r(ied->AddressOfFunctions)))[index]);
      u64 FunAddress = ((u64)mod + FunRVA);

      BOOLEAN IsBoundImport = FALSE;
      ULONG BoundImportNameLenth = 0;
      for (ULONG i = 0; i < 50; i++) {
        u64 pAddr = FunAddress + i;
        UCHAR c = rr<UCHAR>((PUCHAR)pAddr);
        if (c == '.' && i > 0) {
          IsBoundImport = TRUE;
          BoundImportNameLenth = i;
          break;
        } else {
          if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9'))) {
            break;
          }
        }
      }
      // if (IsBoundImport && BoundImportNameLenth < 400) {
      // 	UCHAR BoundImportModuleName[400];
      // 	RtlZeroMemory(BoundImportModuleName,
      // sizeof(BoundImportModuleName));
      // 	m_mmu->read_virt(FunAddress,BoundImportModuleName,BoundImportNameLenth);
      // 	*(ULONG *)(BoundImportModuleName + BoundImportNameLenth) =
      // 'lld.';

      // 	LPCSTR BoundImportFunctionName = (LPCSTR)(FunAddress +
      // BoundImportNameLenth + 1); 	ULONG64 base =
      // (ULONG64)drvbase((PCHAR)BoundImportModuleName); 	if (base) {
      // return ksym((uptr)base, BoundImportFunctionName);
      // 	}
      // }
      return FunAddress;
    }
  }
  return 0;
}

ULONG64 EMU_ExpLookupHandleTableEntry(mmu *m_mmu, ULONG_PTR a1, ULONG64 a2) {
  ULONG64 v2; // rdx
  ULONG64 v3; // r8

  v2 = a2 & 0xFFFFFFFFFFFFFFFC;
  if (v2 >= rr<unsigned int>(a1))
    return 0;
  v3 = rr<ULONG64>((a1 + 8));
  if ((v3 & 3) == 1)
    return rr<ULONG64>(v3 + 8 * (v2 >> 10) - 1) + 4 * (v2 & 0x3FF);
  if ((v3 & 3) != 0)
    return rr<ULONG64>(rr<ULONG64>(v3 + 8 * (v2 >> 19) - 2) +
                       8 * ((v2 >> 10) & 0x1FF)) +
           4 * (v2 & 0x3FF);
  return v3 + 4 * v2;
}
u64 ntfuncs::lookupcid(u64 cid) {
  ULONG64 Entry =
      EMU_ExpLookupHandleTableEntry(m_mmu, rr<u64>(PspCidTable), cid);
  if (Entry == 0)
    return 0;
  // DbgPrint("Entry:%p\n", Entry);
  if (Entry) {
    ULONG64 value = rr<ULONG64>(Entry);
    if (GetBuildNumber() < WIN10) {
      // win7
      ULONG64 mask = ~(ULONG64)7;
      value = value & mask;
      return m_mmu->chkvirt(value) ? value : 0;
    } else {
      value = value >> 0x10;
      if (value == 0)
        return 0;
      value = value | 0xFFFF000000000000;
      return m_mmu->chkvirt(value) ? value : 0;
    }
  }
  return 0;
}
bool ntfuncs::pidexist(int pid) {
  u64 obj = lookupcid(pid);
  if (!m_mmu->chkvirt(obj))
    return false;
  u64 exit_time = rr<u64>(obj + offset_ProcessExitTime);
  if (exit_time != 0)
    return false;
  return true;
}
string ntfuncs::pidname(int pid) {
  u64 eprocess = lookupcid(pid);
  CHAR Name[16] = {0};
  memset(Name, 0, 16);
  m_mmu->read_virt(eprocess + offset_ImageFileName, Name, 15);
  return string(Name);
}
int ntfuncs::enumpid(std::function<bool(ntfuncs *, int)> callback) {
  int pidcount = 0;
  u32 cookie = rr<u32>(ObHeaderCookie);
  u8 typeidx = rr<u8>(rr<u64>(PsProcessType) + 0x28);

  for (int i = 4; i < 0x10000; i += 4) {
    u64 process = lookupcid(i);
    u8 key1 = cookie & 0xFF;
    u8 key2 = (((WORD)((process - 0x30) & 0xFFFF)) >> 8) & 0xFF;
    u8 idx = rr<u8>(process - 0x18);
    idx = idx ^ key1 ^ key2;
    // printf("process:%llX\n", process);
    if (idx == typeidx) {
      // printf("process:%llX\n", process);
      pidcount++;
      if (pidexist(i)) {
        if (!callback(this, i))
          return pidcount;
      }
    }
  }
  return pidcount;
}
std::list<int> ntfuncs::traversepid() {
  std::list<int> r;
  enumpid([&](auto thiz, auto pid) -> bool {
    r.push_back(pid);
    return true;
  });
  return std::move(r);
}
int ntfuncs::findpid(const char *name) {
  if (name == 0)
    return 0;
  int result = 0;

  enumpid([&](auto thiz, auto pid) -> bool {
    string n = thiz->pidname(pid).c_str();
    if (n.length() == 0)
      return true;
    if (!stricmp(n.c_str(), name)) {
      result = pid;
      return false;
    }
    return true;
  });
  return result;
}

typedef struct _UNICODE_STRING {
  WORD Length;
  WORD MaximumLength;
  PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _NON_PAGED_DEBUG_INFO {
  WORD Signature;
  WORD Flags;
  DWORD Size;
  WORD Machine;
  WORD Characteristics;
  DWORD TimeDateStamp;
  DWORD CheckSum;
  DWORD SizeOfImage;
  ULONGLONG ImageBase;
  // DebugDirectorySize
  // IMAGE_DEBUG_DIRECTORY
} NON_PAGED_DEBUG_INFO, *PNON_PAGED_DEBUG_INFO;
typedef struct _KLDR_DATA_TABLE_ENTRY {
  LIST_ENTRY InLoadOrderLinks;
  PVOID ExceptionTable;
  ULONG ExceptionTableSize;
  PVOID GpValue;
  PNON_PAGED_DEBUG_INFO NonPagedDebugInfo;
  PVOID DllBase;
  PVOID EntryPoint;
  ULONG SizeOfImage;
  UNICODE_STRING FullDllName;
  UNICODE_STRING BaseDllName;
  ULONG Flags;
  USHORT LoadCount;
  USHORT __Unused5;
  PVOID SectionPointer;
  ULONG CheckSum;
  PVOID LoadedImports;
  PVOID PatchInformation;
} KLDR_DATA_TABLE_ENTRY, *PKLDR_DATA_TABLE_ENTRY;
#include "peb.h"

int ntfuncs::enumdrv(
    std::function<bool(u64 drvbase, u64 drvsize, pwstr drvname)> callback) {
  PLIST_ENTRY first = (PLIST_ENTRY)PsLoadedModuleList;
  PLIST_ENTRY query = r(first->Flink);
  int drvcount = 0;
  while (first != query) {
    PKLDR_DATA_TABLE_ENTRY ldr = (PKLDR_DATA_TABLE_ENTRY)query;
    PVOID DllBase = r(ldr->DllBase);
    DWORD SizeOfImage = r(ldr->SizeOfImage);
    WCHAR drvname[80];
    memset(drvname, 0, sizeof(drvname));
    int maxlen = r(ldr->BaseDllName.Length) / 2;
    if (maxlen > 78)
      maxlen = 78;
    m_mmu->read_wstring((u64)r(ldr->BaseDllName.Buffer), (pwstr)drvname,
                        maxlen);
    if (!callback((u64)DllBase, SizeOfImage, (pwstr)drvname))
      return drvcount;

    drvcount++;
    query = r(query->Flink);
  }

  return drvcount;
}

u64 ntfuncs::drvbase(const char *name) {
  if (!name)
    return 0;
  WCHAR ws[80] = {0};
  AnsiToUnicode(name, ws, 78);
  u64 result = 0;
  enumdrv([&](u64 drvbase, u64 drvsize, pwstr drvname) -> bool {
    if (!wcsicmp((PWSTR)drvname, ws)) {
      result = drvbase;
      return false;
    }
    return true;
  });
  return result;
}
u64 ntfuncs::drvsize(const char *name) {
  if (!name)
    return 0;
  WCHAR ws[80] = {0};
  AnsiToUnicode(name, ws, 78);
  u64 result = 0;
  enumdrv([&](u64 drvbase, u64 drvsize, pwstr drvname) -> bool {
    if (!wcsicmp((PWSTR)drvname, ws)) {
      result = drvsize;
      return false;
    }
    return true;
  });
  return result;
}

std::shared_ptr<process> ntfuncs::p(int pid) {
  if (!pidexist(pid))
    return nullptr;
  u64 eprocess = lookupcid(pid);
  if (!eprocess)
    return nullptr;
  u64 dtb = GetDirectoryTableBase(eprocess);
  if (!dtb)
    return nullptr;
  mmu_initializer _m(*m_mmu, dtb);
  return std::make_shared<process>(this, _m, eprocess, pid);
}

int process::BalancedNodeTraverse(u64 Node,
                                  std::function<bool(u64 Node)> callback) {
  auto pdb = sys()->allpdbs();
  auto &&ntos = (*pdb)["ntoskrnl.exe"];

  int offset_Children =
      ntos.structures["_RTL_BALANCED_NODE"]["Children"].offset;
  int offset_Left = ntos.structures["_RTL_BALANCED_NODE"]["Left"].offset;
  int offset_Right = ntos.structures["_RTL_BALANCED_NODE"]["Right"].offset;
  int offset_ParentValue =
      ntos.structures["_RTL_BALANCED_NODE"]["ParentValue"].offset;

  if (!chkvirt(Node))
    return 0;
  callback(Node);
  u64 Left = rr<u64>(Node + offset_Left);
  u64 Right = rr<u64>(Node + offset_Right);

  int result = 0;
  result += BalancedNodeTraverse(Left, callback);
  result += BalancedNodeTraverse(Right, callback);

  return result;
}

int process::enummod(
    std::function<bool(u64 base, u64 size, pwstr name)> callback) {
  int mod_cnt = 0;
  PMYPEB peb = (PMYPEB)sys()->GetPeb(_eprocess);

  if (peb && m_mmu->chkvirt(peb)) {
    PPEB_LDR_DATA ldr = r(peb->Ldr);
    if (!m()->chkvirt(ldr))
      return 0;
    PLIST_ENTRY first = &ldr->InLoadOrderModuleList;
    PLIST_ENTRY query = (PLIST_ENTRY)r(first->Blink);
    if (!m()->chkvirt(query))
      return 0;
    while (first != query) {
      if (!m()->chkvirt(query) ||
          !m()->chkvirt(((PUCHAR)query + sizeof(LDR_DATA_TABLE_ENTRY) - 1)))
        return 0;
      PLDR_DATA_TABLE_ENTRY data = (PLDR_DATA_TABLE_ENTRY)query;

      PUNICODE_STRING Name = &data->FullDllName;
      if (Name && m()->chkvirt(Name)) {
        PVOID NameBuffer = (PVOID)r(Name->Buffer);
        USHORT NameLength = r(Name->Length);

        if (NameBuffer && m()->chkvirt(NameBuffer) && NameLength &&
            m()->chkvirt((NameLength + ((PUCHAR)NameBuffer) - 1))) {
          WCHAR wName[80];
          memset(wName, 0, sizeof(wName));
          DWORD ReadLen = NameLength / 2;
          if (ReadLen > 78)
            ReadLen = 78;
          if (m()->read_wstring((u64)NameBuffer, (pwstr)wName, ReadLen)) {
            LPCWSTR sufMooduleName = GetFileName(wName);
            mod_cnt++;
            if (!callback((u64)r(data->DllBase), r(data->SizeOfImage),
                          (pwstr)sufMooduleName)) {
              return mod_cnt;
            }
          }
        }
      } else {
        return mod_cnt;
      }
      mod_cnt++;
      query = (PLIST_ENTRY)r(query->Blink);
    }
  }
  return mod_cnt;
}
int process::enummod32(
    std::function<bool(u64 base, u64 size, pwstr name)> callback) {
  PMYPEB32 peb = (PMYPEB32)sys()->GetPeb32(_eprocess);
  int mod_cnt = 0;
  PVOID Base = 0;

  if (peb && m_mmu->chkvirt(peb)) {

    PPEB_LDR_DATA32 ldr = (PPEB_LDR_DATA32)r(peb->Ldr);
    PLIST_ENTRY32 first = &ldr->InLoadOrderModuleList;
    PLIST_ENTRY32 query = (PLIST_ENTRY32)r(first->Blink);

    while (first != query) {
      if (!m()->chkvirt(query) ||
          !m()->chkvirt(((PUCHAR)query + sizeof(LDR_DATA_TABLE_ENTRY32) - 1)))
        return 0;
      PLDR_DATA_TABLE_ENTRY32 data = (PLDR_DATA_TABLE_ENTRY32)query;

      PMYUNICODE_STRING32 Name = &data->BaseDllName;
      if (Name && m()->chkvirt(Name)) {
        ULONG NameBuffer = r(Name->Buffer);
        USHORT NameLength = r(Name->Length);
        if (NameBuffer && m()->chkvirt((PVOID)NameBuffer) && NameLength &&
            m()->chkvirt((NameLength + ((PUCHAR)NameBuffer) - 1))) {
          WCHAR wName[80];
          RtlZeroMemory(wName, sizeof(wName));
          DWORD ReadLen = NameLength / 2;
          if (ReadLen > 78)
            ReadLen = 78;
          if (m()->read_wstring((DWORD64)NameBuffer, (pwstr)wName, ReadLen)) {
            mod_cnt++;
            LPCWSTR sufMooduleName = GetFileName(wName);
            if (!callback((u64)r(data->DllBase), r(data->SizeOfImage),
                          (pwstr)sufMooduleName)) {
              return mod_cnt;
            }
          }
        }
      }
      query = (PLIST_ENTRY32)r(query->Blink);
    }
  }
  return mod_cnt;
}
int process::enumvad(
    std::function<bool(u64 base, u64 size, u64 prot, u64 type)> callback) {
  // int VadRoot;
  auto pdb = sys()->allpdbs();
  if (pdb != nullptr) {
    auto &&ntos = (*pdb)["ntoskrnl.exe"];
    int offset_VadRoot = ntos.structures["_EPROCESS"]["VadRoot"].offset;
    int offset_Root = ntos.structures["_RTL_AVL_TREE"]["Root"].offset;

    int StartingVpn = ntos.structures["_MMVAD_SHORT"]["StartingVpn"].offset;
    int EndingVpn = ntos.structures["_MMVAD_SHORT"]["EndingVpn"].offset;
    int StartingVpnHigh =
        ntos.structures["_MMVAD_SHORT"]["StartingVpnHigh"].offset;
    int EndingVpnHigh = ntos.structures["_MMVAD_SHORT"]["EndingVpnHigh"].offset;
    int Flags = ntos.structures["_MMVAD_SHORT"]["u1"].offset;

    auto field_VadType = ntos.structures["_MMVAD_FLAGS"]["VadType"];
    auto field_Protection = ntos.structures["_MMVAD_FLAGS"]["Protection"];
    auto field_PrivateMemory = ntos.structures["_MMVAD_FLAGS"]["PrivateMemory"];

    u64 MmProtectToValue =
        sys()->GetNtoskrnl() + ntos.symbols["MmProtectToValue"];

    struct {
      int StartingVpn;
      int EndingVpn;
      int StartingVpnHigh;
      int EndingVpnHigh;
      int Flags;
      decltype(field_VadType) VadType;
      decltype(field_Protection) Protection;
      decltype(field_PrivateMemory) PrivateMemory;
      int MmProtectToValue[32];
    } ctx;
    ctx.StartingVpn = StartingVpn;
    ctx.EndingVpn = EndingVpn;
    ctx.StartingVpnHigh = StartingVpnHigh;
    ctx.EndingVpnHigh = EndingVpnHigh;
    ctx.Flags = Flags;
    ctx.VadType = field_VadType;
    ctx.Protection = field_Protection;
    ctx.PrivateMemory = field_PrivateMemory;
    for (int i = 0; i < 32; i++) {
      ctx.MmProtectToValue[i] = rr<int>(MmProtectToValue + 4 * i);
    }

    u64 FirstNode = rr<u64>(eprocess() + offset_VadRoot + offset_Root);
    BalancedNodeTraverse(FirstNode, [&](u64 Node) -> bool {
      auto r_ctx = &ctx;
      auto StartingVpn = this->r<u32>(Node + r_ctx->StartingVpn);
      auto EndingVpn = this->r<u32>(Node + r_ctx->EndingVpn);
      auto StartingVpnHigh = this->r<u8>(Node + r_ctx->StartingVpnHigh);
      auto EndingVpnHigh = this->r<u8>(Node + r_ctx->EndingVpnHigh);
      auto Flags = this->r<u32>(Node + r_ctx->Flags);

      int VadType = GetBitFieldValue(Flags, r_ctx->VadType.bitfield_offset,
                                     r_ctx->VadType.bitfield_offset +
                                         r_ctx->VadType.bitfield_length - 1);
      int Protection =
          GetBitFieldValue(Flags, r_ctx->Protection.bitfield_offset,
                           r_ctx->Protection.bitfield_offset +
                               r_ctx->Protection.bitfield_length - 1);
      int PrivateMemory =
          GetBitFieldValue(Flags, r_ctx->PrivateMemory.bitfield_offset,
                           r_ctx->PrivateMemory.bitfield_offset +
                               r_ctx->PrivateMemory.bitfield_length - 1);

      u64 StartingVirtualAddress =
          StartingVpn | ((static_cast<u64>(StartingVpnHigh)) << 32);
      EndingVpn += 1;
      u64 EndingVirtualAddress =
          EndingVpn | ((static_cast<u64>(EndingVpnHigh)) << 32);

      StartingVirtualAddress *= 0x1000;
      EndingVirtualAddress *= 0x1000;
      u64 size = EndingVirtualAddress - StartingVirtualAddress;

      auto convertProtection = [&](u32 vadprot) -> u32 {
        vadprot &= 0x1f;
        return r_ctx->MmProtectToValue[vadprot];
      };

      u32 pageprot = convertProtection(Protection);
      p1x(pageprot);
      p1x(Protection);
      p1d(VadType);
      u32 type = PrivateMemory == 1 ? MEM_PRIVATE : 0;
      type |= VadType == VadImageMap ? MEM_IMAGE | MEM_MAPPED | MEM_COMMIT : 0;
      type |= VadType == VadDevicePhysicalMemory ? MEM_MAPPED | MEM_COMMIT : 0;
      type |= VadType == VadRotatePhysical ? MEM_COMMIT : 0;

      callback(StartingVirtualAddress, size, pageprot, type);

      return true;
    });
  } else {
    return 0;
  }
  return 0;
}

std::list<std::tuple<u64, u64, u64, u64>> process::traversevad() {
  std::list<std::tuple<u64, u64, u64, u64>> r;
  enumvad([&](u64 base, u64 size, u64 prot, u64 type) -> bool {
    r.push_back({base, size, prot, type});
  });
  return r;
}

std::list<std::tuple<u64, u64, std::string>> process::traversemod() {
  std::list<std::tuple<u64, u64, std::string>> r;
  enummod([&](u64 base, u64 size, pwstr name) -> bool {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    p1x(name);
    r.push_back(std::make_tuple(base, size, converter.to_bytes(name)));
    return true;
  });
  return std::move(r);
}
std::list<std::tuple<u64, u64, std::string>> process::traversemod32() {
  std::list<std::tuple<u64, u64, std::string>> r;
  enummod32([&](u64 base, u64 size, pwstr name) -> bool {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    r.push_back(std::make_tuple(base, size, converter.to_bytes(name)));
    return true;
  });
  return std::move(r);
}
u64 process::sym(uptr mod, const char *symname) {
  IMAGE_DOS_HEADER *idh = (IMAGE_DOS_HEADER *)mod;
  IMAGE_NT_HEADERS64 *inh = (IMAGE_NT_HEADERS64 *)(r(idh->e_lfanew) + (u64)idh);
  IMAGE_EXPORT_DIRECTORY *ied =
      (IMAGE_EXPORT_DIRECTORY
           *)((u64)mod +
              r(inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
                    .VirtualAddress));

  DWORD NumberOfNames = r(ied->NumberOfNames);
  for (int i = 0; i < NumberOfNames; i++) {
    USHORT index =
        rr<USHORT>(&((USHORT *)((u64)mod + r(ied->AddressOfNameOrdinals)))[i]);
    ULONG NameRVA =
        rr<ULONG>(&((ULONG *)((u64)mod + r(ied->AddressOfNames)))[i]);
    PCSTR Name = (PCSTR)(((ULONG64)mod) + NameRVA);
    CHAR cName[40] = {};
    memset(cName, 0, 40);
    m_mmu->read_string((u64)Name, cName, 40);

    if (!strcmp(cName, symname)) {
      ULONG FunRVA =
          rr<ULONG>(&((ULONG *)((u64)mod + r(ied->AddressOfFunctions)))[index]);
      u64 FunAddress = ((u64)mod + FunRVA);

      BOOLEAN IsBoundImport = FALSE;
      ULONG BoundImportNameLenth = 0;
      for (ULONG i = 0; i < 50; i++) {
        u64 pAddr = FunAddress + i;
        UCHAR c = rr<UCHAR>((PUCHAR)pAddr);
        if (c == '.' && i > 0) {
          IsBoundImport = TRUE;
          BoundImportNameLenth = i;
          break;
        } else {
          if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9'))) {
            break;
          }
        }
      }

      return FunAddress;
    }
  }
  return 0;
}

u64 process::modbase(const char *name) {
  if (!name)
    return 0;
  WCHAR ws[80] = {0};
  memset(ws, 0, sizeof(ws));
  AnsiToUnicode(name, ws, 78);
  u64 result = 0;
  enummod([&](u64 base, u64 size, pwstr name) -> bool {
    if (!wcsicmp((PWSTR)name, ws)) {
      result = base;
      return false;
    }
    return true;
  });
  return result;
}
u64 process::modsize(const char *name) {
  if (!name)
    return 0;
  WCHAR ws[80] = {0};
  memset(ws, 0, sizeof(ws));
  AnsiToUnicode(name, ws, 78);
  u64 result = 0;
  enummod([&](u64 base, u64 size, pwstr name) -> bool {
    if (!wcsicmp((PWSTR)name, ws)) {
      result = size;
      return false;
    }
    return true;
  });
  return result;
}
u64 process::modbase32(const char *name) {
  if (!name)
    return 0;
  WCHAR ws[80] = {0};
  memset(ws, 0, sizeof(ws));
  AnsiToUnicode(name, ws, 78);
  u64 result = 0;
  enummod32([&](u64 base, u64 size, pwstr name) -> bool {
    if (!wcsicmp((PWSTR)name, ws)) {
      result = base;
      return false;
    }
    return true;
  });
  return result;
}
u64 process::modsize32(const char *name) {
  if (!name)
    return 0;
  WCHAR ws[80] = {0};
  memset(ws, 0, sizeof(ws));
  AnsiToUnicode(name, ws, 78);
  u64 result = 0;
  enummod32([&](u64 base, u64 size, pwstr name) -> bool {
    if (!wcsicmp((PWSTR)name, ws)) {
      result = size;
      return false;
    }
    return true;
  });
  return result;
}
u64 process::sectionbase() {
  return this->sys()->GetSectionBaseAddress(this->eprocess());
}
