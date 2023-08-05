
#ifndef _DMA_NT_UTIL_H_
#define _DMA_NT_UTIL_H_

#include "dma_memsch.h"
#include "dma_mmu.h"
#include "dma_peparser.h"
#include "dma_symbol.h"
#include "dma_type.h"
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>

class process;

struct pdbinfo {
  std::shared_ptr<dma_symbol_interface> interface;
  std::map<std::string, std::map<std::string, dma_field_info>> structures;
  std::map<std::string, int64_t> symbols;
};

class ntfuncs {

public:
  ntfuncs(const mmu_initializer &_m, u64 ntos,
          std::shared_ptr<dma_symbol_factory> symbol_factor = nullptr);
  ~ntfuncs();
  mmu *getmmu() { return m_mmu; }
  bool is_valid();

  std::shared_ptr<std::map<std::string, pdbinfo>> allpdbs();
  u64 kscanmem(uptr virt, u64 sz, const char *pattern) {
    u64 pos = dma_memsch::sd_search(m_mmu, virt, sz, pattern);
    return pos == -1 ? 0 : virt + pos;
  }
  u64 ksectaddr(uptr mod, const char *sectname) {

    return peMemory64(*m_mmu, mod).sectaddr(sectname);
  }
  u64 ksectsize(uptr mod, const char *sectname) {
    return peMemory64(*m_mmu, mod).sectsize(sectname);
  }
  u64 kscansect(uptr mod, const char *sectname, const char *pattern) {
    return peMemory64(*m_mmu, mod).scansect(sectname, pattern);
  }
  u64 kfuncbegin(uptr mod, uptr pc) {
    return peMemory64(*m_mmu, mod).funcbegin(pc);
  }
  u64 kfuncend(uptr mod, uptr pc) {
    return peMemory64(*m_mmu, mod).funcend(pc);
  }
  u64 ksym(uptr mod, const char *symname);
  u64 lookupcid(u64 cid);
  bool pidexist(int pid);
  std::string pidname(int pid);
  int findpid(const char *name);
  // returns true to continue enumerating
  int enumpid(std::function<bool(ntfuncs *, int)> callback);
  std::list<int> traversepid();
  // returns true to continue enumerating
  int enumdrv(
      std::function<bool(u64 drvbase, u64 drvsize, pwstr drvname)> callback);
  u64 drvbase(const char *name);
  u64 drvsize(const char *name);

  u32 GetBuildNumber() { return BuildNumber; }
  uptr GetNtoskrnl() { return ntoskrnl; }

  u64 GetSectionBaseAddress(u64 eprocess) {
    return m_mmu->r<u64>(offset_SectionBaseAddress + eprocess);
  }
  u64 GetPeb(u64 eprocess) { return m_mmu->r<u64>(offset_Peb + eprocess); }
  u64 GetPeb32(u64 eprocess) {
    return m_mmu->r<u64>(m_mmu->r<u64>(offset_WoW64Process + eprocess));
  }
  u64 GetDirectoryTableBase(u64 eprocess) {
    return m_mmu->r<u64>(offset_DirectoryTableBase + eprocess);
  }
  std::shared_ptr<process> p(int pid);

private:
  bool m_valid = false;
  std::shared_ptr<dma_symbol_factory> m_symbol_factory = nullptr;
  mmu *m_mmu = 0;

  std::shared_ptr<std::map<std::string, pdbinfo>> m_allpdbs;

  u64 KdDebuggerDataBlock = 0;
  u64 PspCidTable = 0;
  u64 KiProcessorBlock = 0;
  u64 PsLoadedModuleList = 0;
  u64 PsActiveProcessHead = 0;
  u64 ObHeaderCookie = 0;
  u64 PsProcessType = 0;

  u64 BuildNumber = 0;
  u64 ntoskrnl = 0;

  u64 offset_SectionBaseAddress = 0;
  u64 offset_Peb = 0;
  u64 offset_WoW64Process = 0;
  u64 offset_DirectoryTableBase = 0;
  u64 offset_ImageFileName = 0;
  u64 offset_ProcessExitTime = 0;

  u64 win32k = 0;
  u64 win32kbase = 0;
  u64 win32kfull = 0;

  u64 gafAsyncKeyState = 0;
  u64 gptCursorAsync = 0;
  bool initialize_without_symbol();
  bool initialize_with_symbol();
};

class process : public mmu {
private:
  ntfuncs *ntf = 0;
  u64 _eprocess = 0;
  int _pid = 0;
  mmu *m_mmu = nullptr;
  mmu *m() { return m_mmu; }
  int BalancedNodeTraverse(u64 Node, std::function<bool(u64 Node)> callback);

public:
  process(ntfuncs *ntf, const mmu_initializer &_m, u64 eprocess, int pid)
      : mmu(_m) {
    this->ntf = ntf;
    this->_eprocess = eprocess;
    this->_pid = pid;
    m_mmu = this;
  }
  ntfuncs *sys() { return ntf; }
  int enummod(std::function<bool(u64 base, u64 size, pwstr name)> callback);
  int enummod32(std::function<bool(u64 base, u64 size, pwstr name)> callback);
  int enumvad(
      std::function<bool(u64 base, u64 size, u64 prot, u64 type)> callback);
  std::list<std::tuple<u64, u64, u64, u64>> traversevad();

  // tuple element : base size name
  std::list<std::tuple<u64, u64, std::string>> traversemod();
  std::list<std::tuple<u64, u64, std::string>> traversemod32();
  u64 sym(uptr mod, const char *symname);
  u64 modbase(const char *name);
  u64 modsize(const char *name);
  u64 modbase32(const char *name);
  u64 modsize32(const char *name);
  u64 eprocess() { return this->_eprocess; }
  int pid() { return _pid; }
  u64 sectionbase();
  bool isactive() { return sys()->pidexist(_pid); }
};

#endif // !_DMA_NT_UTIL_H_
