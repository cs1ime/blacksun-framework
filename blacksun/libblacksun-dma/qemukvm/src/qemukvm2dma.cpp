#include <qemukvm2dma.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <dirent.h>
#include <dma.h>
#include <dma_type.h>
#include <fcntl.h>
#include <fmt/core.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <regex>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <tuple>
#include <utility>
#include <string>


#define printf
#define xs

static bool qemukvm_run_system_command(const char *cmd, std::string &output) {
  std::string result = "";
  char buf[1024] = {0};
  FILE *fp = NULL;

  if ((fp = popen(cmd, xs("r"))) == NULL) {
    perror(xs("popen error!\n"));
    return false;
  }
  memset(buf, 0, sizeof(buf));
  while (fgets(buf, sizeof(buf) - 1, fp)) {
    result += buf;
    memset(buf, 0, sizeof(buf));
  }
  int stats = 0;
  wait(&stats);
  pclose(fp);

  std::string code_ = std::string(result);
  output = std::string(result);
  return stats == 0;
}
static void qemukvm_run_system_command_mustsuccess(const char *cmd,
                                                   std::string &output) {
  if (!qemukvm_run_system_command(cmd, output)) {
    perror(xs("occured error!"));
    exit(1);
  }
}
static void qemukvm_process_enummem(
    pid_t pid,
    std::function<bool(const char *name, uint64_t addr, uint64_t imgsz)>
        callback) {
  char path_proc_pid_maps[1024] = {0};
  snprintf(path_proc_pid_maps, 1024, xs("/proc/%d/maps"), pid);
  FILE *fp = fopen(path_proc_pid_maps, xs("r"));
  if (!fp)
    return;

  char line[1024];
  struct {
    int inode = 0;
    uint64_t baseaddr = 0;
    uint64_t endaddr = 0;
    char path[1024] = {0};
  } lookinginode;
  bool looked = false;
  // int lookinginode=0;
  while (fgets(line, 1024, fp) != 0) {
    uint64_t addr_begin = 0, addr_end = 0;
    uint32_t offset = 0;
    uint32_t dev_major = 0, dev_minor = 0;
    int inode = 0;
    char pathname[1024] = {0};

    char prot[5] = {0};
    sscanf(line, "%llx-%llx %4s %x %x:%x %d %1024s", &addr_begin, &addr_end,
           prot, &offset, &dev_major, &dev_minor, &inode, &pathname);
    if (prot[0] == 'r' && prot[1] == 'w' && prot[3] == 'p') {
      uint64_t memsz = addr_end - addr_begin;
      if (!callback(pathname, addr_begin, memsz))
        return;
    }
  }
}
static std::pair<uint64_t, uint64_t>
qemukvm_process_get_biggest_memblock(pid_t pid) {
  uint64_t biggest_address = 0;
  uint64_t biggest_size = 0;
  qemukvm_process_enummem(
      pid, [&](const char *name, uint64_t addr, uint64_t imgsz) -> bool {
        if (biggest_size < imgsz) {
          biggest_size = imgsz;
          biggest_address = addr;
        }
        return true;
      });
  return std::pair<uint64_t, uint64_t>(biggest_address, biggest_size);
}
static std::list<std::tuple<uint64_t, uint64_t, uint64_t>>
qemukvm_parse_mtree_string(std::string str) {
  std::istringstream iss(str);
  std::string line;
  std::list<std::tuple<uint64_t, uint64_t, uint64_t>> memory_regions;
  bool flag_startrec = false;
  while (std::getline(iss, line)) {
    std::smatch match;
    if (!flag_startrec) {
      std::regex system_pattern(xs("Root memory region: (.+)"));
      if (std::regex_search(line, match, system_pattern)) {
        if (!match.str(1).compare(xs("system"))) {

          flag_startrec = true;
        }
      }
    } else {
      if (line.empty()) {
        break;
      }

      std::regex regex_zeroaddress(
          xs(R"(([0-9a-fA-F]+)-([0-9a-fA-F]+) ([^:]+): pc\.ram KVM)"));
      std::regex regex_addressable(xs(
          R"(([0-9a-fA-F]+)-([0-9a-fA-F]+) ([^:]+): pc\.ram @([0-9a-fA-F]+) KVM)"));

      std::regex regex_available(xs(R"(([0-9a-fA-F]+)-([0-9a-fA-F]+))"));

      auto record_range = [&](uint64_t phys_start, uint64_t phys_end,
                              uint64_t virt_start) {
        if (phys_end <= phys_start) {
          return;
        }
        uint64_t phys_size = phys_end - phys_start + 1;
        memory_regions.push_back(std::tuple<uint64_t, uint64_t, uint64_t>(
            phys_start, phys_size, virt_start));
      };

      if (std::regex_search(line, match, regex_zeroaddress)) {
        std::string phys_start = match.str(1);
        std::string phys_end = match.str(2);

        record_range(strtoul(phys_start.c_str(), 0, 16),
                     strtoul(phys_end.c_str(), 0, 16), 0);
      } else if (std::regex_search(line, match, regex_addressable)) {
        std::string phys_start = match.str(1);
        std::string phys_end = match.str(2);
        std::string virt_start = match.str(4);

        record_range(strtoul(phys_start.c_str(), 0, 16),
                     strtoul(phys_end.c_str(), 0, 16),
                     strtoul(virt_start.c_str(), 0, 16));
      } else if (!std::regex_search(line, match, regex_available)) {
        break;
      }
    }
  }
  return std::move(memory_regions);
}
static int qemukvm_system_foreach_cmdline(
    std::function<bool(int pid, const std::vector<char> &cmdline)> callback) {
  DIR *dir = opendir(xs("/proc"));
  if (!dir) {
    perror(xs("opendir failed"));
    return 0;
  }
  struct dirent *entry;
  while ((entry = readdir(dir))) {
    if (entry->d_type != DT_DIR) {
      continue;
    }
    const char *name = entry->d_name;
    if (*name < '0' || *name > '9') {
      continue;
    }
    int pid = atoi(name);
    char cmdline_path[256];
    snprintf(cmdline_path, sizeof(cmdline_path), xs("/proc/%d/cmdline"), pid);
    FILE *cmdline_file = fopen(cmdline_path, xs("r"));
    if (!cmdline_file) {
      continue;
    }
    char cmdline[256] = {};
    std::vector<char> vec_cmdline;
    while (!feof(cmdline_file)) {
      int len = fread(cmdline, 1, sizeof(cmdline), cmdline_file);
      vec_cmdline.insert(vec_cmdline.end(), cmdline, cmdline + len);
    }
    fclose(cmdline_file);
    if (!callback(pid, vec_cmdline)) {
      closedir(dir);
      return 0;
    }
  }
  closedir(dir);
  return 0;
}
static pid_t qemukvm_find_pid(std::string vm_name) {
  pid_t result = 0;
  qemukvm_system_foreach_cmdline(
      [&](auto pid, const std::vector<char> &_cmdline)->bool {
        if(_cmdline.empty())
          return true;
        std::vector<char> cmdline(_cmdline);
        for (char &c : cmdline) {
          if (c == '\0')
            c = ' ';
        }
        cmdline.push_back(0);
        boost::regex regex_name("(?<=-name )[^ ]*");
        boost::smatch match;
        if (boost::regex_search(std::string(cmdline.data()), match, regex_name)) {
          std::string _name = match.str(0);
          if (vm_name == _name) {
            result = pid;
            return true;
          }
          std::vector<std::string> subargs;
          boost::algorithm::split(subargs,_name,boost::is_any_of(","));
          for (const auto &_subarg : subargs) {
            auto &&s=_subarg;
            std::vector<std::string> _param_assigning_array;
            boost::algorithm::split(_param_assigning_array,s,boost::is_any_of("="));
            if (!_param_assigning_array.empty()) {
              if (_param_assigning_array.size() >= 2 &&
                  _param_assigning_array[0] == "guest") {
                if (_param_assigning_array[1] == vm_name) {
                  result = pid;
                  return false;
                }
              }
            }
          }
        }
        return true;
      });
  return result;
}

class memmap {
private:
  uint64_t m_maximum_memory_size;
  uint64_t *m_physidx_to_virtoffset;
  uint32_t *m_physical_memmap;

public:
  explicit memmap(std::string mtree_output) {

    std::list<std::tuple<uint64_t, uint64_t, uint64_t>> memory_regions =
        qemukvm_parse_mtree_string(mtree_output);
    auto maximum =
        std::max_element(memory_regions.begin(), memory_regions.end(),
                         [](auto &low, auto &high) -> bool {
                           return (std::get<0>(low) + std::get<1>(low)) <
                                  (std::get<1>(high) + std::get<0>(high));
                         });

    if (maximum != memory_regions.end()) {
      auto maximum_physical_address =
          std::get<0>(*maximum) + std::get<1>(*maximum);
      // printf("maximum_physical_address : %llX\r\n",maximum_physical_address);

      auto numAlloc = maximum_physical_address / 0x1000;
      auto szAlloc = numAlloc * sizeof(uint32_t);
      auto physical_memmap = (uint32_t *)malloc(szAlloc);
      if (physical_memmap == nullptr)
        return;
      memset(physical_memmap, 0, szAlloc);
      szAlloc = memory_regions.size() * sizeof(uint64_t) * 2;
      auto physidx_to_virtoffset = (uint64_t *)malloc(szAlloc);
      if (physidx_to_virtoffset == nullptr) {
        free(physical_memmap);
        return;
      }
      memset(physidx_to_virtoffset, 0, szAlloc);
      if (physical_memmap != nullptr && physidx_to_virtoffset != nullptr) {
        uint32_t current_index = 0;
        for (auto &bl : memory_regions) {
          auto physical_address = std::get<0>(bl);
          auto physical_size = std::get<1>(bl);

          auto physicstartidx = physical_address / 0x1000;
          for (uint64_t i = 0; i < physical_size / 0x1000; i++) {
            physical_memmap[physicstartidx + i] = current_index + 1;
          }
          physidx_to_virtoffset[current_index * 2] = std::get<0>(bl);
          physidx_to_virtoffset[current_index * 2 + 1] = std::get<2>(bl);
          current_index++;
        }

        m_physical_memmap = physical_memmap;
        m_physidx_to_virtoffset = physidx_to_virtoffset;
        m_maximum_memory_size = maximum_physical_address;
      }
    }
  }
  ~memmap() {
    if (m_physidx_to_virtoffset) {
      free(m_physidx_to_virtoffset);
      m_physidx_to_virtoffset = 0;
    }

    if (m_physical_memmap) {
      free(m_physical_memmap);
      m_physical_memmap = 0;
    }
  }
  bool valid() {
    if (!m_physidx_to_virtoffset || !m_physical_memmap) {
      return false;
    }
    return true;
  }
  uint64_t lookup_physical_adress(uint64_t physical_address) {
    if (!m_physidx_to_virtoffset || !m_physical_memmap) {
      return static_cast<uint64_t>((uint64_t)-1);
    }
    if (physical_address > m_maximum_memory_size) {
      return static_cast<uint64_t>((uint64_t)-1);
    }
    uint64_t lookup_index = physical_address / 0x1000;
    uint32_t idx = m_physical_memmap[lookup_index];
    if (idx == 0) {
      return static_cast<uint64_t>((uint64_t)-1);
    }
    idx--;
    uint64_t start_physaddr = m_physidx_to_virtoffset[idx * 2];
    uint64_t start_virtaddr = m_physidx_to_virtoffset[idx * 2 + 1];

    return physical_address - start_physaddr + start_virtaddr;
  }
};

class kvmdma {
private:
  pid_t m_pid = 0;
  uint64_t m_address = 0;
  uint64_t m_memory_size = 0;

  std::shared_ptr<memmap> m_memmap = nullptr;

public:
  explicit kvmdma(pid_t _pid, std::shared_ptr<memmap> mp) {
    auto [_address, _memory_size]  =
        qemukvm_process_get_biggest_memblock(_pid);
    printf("addr:%llx size:%llx\r\n", _address, _memory_size);
    this->m_pid = _pid;
    this->m_address = _address;
    this->m_memory_size = _memory_size;
    this->m_memmap = mp;
  }
  ~kvmdma() {}
  bool valid() { return (this->m_address != 0 && this->m_memory_size != 0); }
  bool read_physical_memory(physaddr addr, u8 *buffer, size_t size) {
    uint64_t virt_offset = this->m_memmap->lookup_physical_adress(addr);
    if (virt_offset == static_cast<uint64_t>((uint64_t)-1)) {
      return false;
    }
    if (size == 0)
      return true;

    struct iovec iov[1];
    iov->iov_base = (void *)(virt_offset + m_address);
    iov->iov_len = size;

    struct iovec riov[1];
    riov->iov_base = buffer;
    riov->iov_len = size;
    size_t r = process_vm_readv(m_pid, riov, 1, iov, 1, 0);
    // puts(strerror(errno));
    return errno == 0 && r > 0;
  }
  bool write_physical_memory(physaddr addr, u8 *buffer, size_t size) {
    uint64_t virt_offset = this->m_memmap->lookup_physical_adress(addr);
    if (virt_offset == static_cast<uint64_t>((uint64_t)-1)) {
      return false;
    }
    if (size == 0)
      return true;

    struct iovec iov[1];
    iov->iov_base = (void *)(virt_offset + m_address);
    iov->iov_len = size;

    struct iovec riov[1];
    riov->iov_base = buffer;
    riov->iov_len = size;
    size_t r = process_vm_writev(m_pid, riov, 1, iov, 1, 0);
    // puts(strerror(errno));
    return errno == 0 && r > 0;
  }
};

class qemukvm2dma_impl {
public:
  explicit qemukvm2dma_impl(std::string vm_name) {
    auto pid = qemukvm_find_pid(vm_name);
    if (pid == 0)
      return;
    std::string out;
    if (!qemukvm_run_system_command(
            fmt::format("virsh qemu-monitor-command {} --hmp info mtree -f",
                        vm_name).c_str(),
            out))
      return;
    m_memmap = std::make_shared<memmap>(out);
    if (!m_memmap->valid())
      return;
    m_kvmdma = std::make_shared<kvmdma>(pid, m_memmap);
    if (!m_kvmdma->valid())
      return;
    m_valid = true;
  }
  bool valid() { return m_valid; }
  bool read_physical_memory(physaddr pa, u8 *pb, size_t cb) {
    if (pa == D_BADPHYSADDR)
      return false;
    return m_kvmdma->read_physical_memory(pa, pb, cb);
  }
  bool write_physical_memory(physaddr pa, u8 *pb, size_t cb) {

    if (pa == D_BADPHYSADDR)
      return false;
    return m_kvmdma->write_physical_memory(pa, pb, cb);
  }

private:
  bool m_valid = false;
  std::shared_ptr<memmap> m_memmap;
  std::shared_ptr<kvmdma> m_kvmdma;
};

qemukvm2dma::qemukvm2dma(std::string vm) {
  impl = std::make_shared<qemukvm2dma_impl>(vm);
}
bool qemukvm2dma::valid() { return impl->valid(); }
bool qemukvm2dma::read_physical_memory(physaddr pa, u8 *pb, size_t cb) {
  return impl->read_physical_memory(pa, pb, cb);
}
bool qemukvm2dma::write_physical_memory(physaddr pa, u8 *pb, size_t cb) {
  return impl->write_physical_memory(pa, pb, cb);
}
