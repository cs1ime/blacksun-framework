#include "rawmem2dma.h"
#include <dirent.h>
#include <dma_type.h>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <tuple>
#include <unistd.h>
#include <utility>

#define printf

rawmem2dma::rawmem2dma(std::string filename) {
  if (filename.empty())
    return;
  int fd = open(filename.c_str(), O_RDONLY);
  if (fd != -1) {
    struct stat st = {};
    if (stat(filename.c_str(), &st) == 0) {
      void *addr = mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
      if (addr != nullptr) {
        m_mapped_addr = addr;
        m_mapped_size = st.st_size;
        m_inited = true;
      } else {
        perror("mmap");
      }
    } else {
      perror("stat");
    }
    close(fd);
  } else {
    perror("open");
  }
}
rawmem2dma::~rawmem2dma() {
  if (m_mapped_addr != nullptr) {
    munmap(m_mapped_addr, m_mapped_size);
    m_mapped_addr = nullptr;
    m_mapped_size = 0;
  }
  if (m_rawmemfd != 0) {
    close(m_rawmemfd);
    m_rawmemfd = 0;
  }
}
bool rawmem2dma::read_physical_memory(physaddr pa, u8 *pb, size_t cb) {
  if (!rawmem2dma::m_mapped_addr || rawmem2dma::m_mapped_size == 0) {
    return false;
  }
  uint64_t realsize = pa + cb;
  if (realsize > rawmem2dma::m_mapped_size) {
    return false;
  }
  if (realsize == 0) {
    return false;
  }
  if (pa == D_BADPHYSADDR)
    return false;
  memcpy(pb, pa + (unsigned char *)rawmem2dma::m_mapped_addr, cb);
  return true;
}
bool rawmem2dma::write_physical_memory(physaddr pa, u8 *pb, size_t cb) {
  if (!rawmem2dma::m_mapped_addr || rawmem2dma::m_mapped_size == 0) {
    return false;
  }
  uint64_t realsize = pa + cb;
  if (realsize > rawmem2dma::m_mapped_size) {
    realsize = rawmem2dma::m_mapped_size;
  }
  if (realsize == 0) {
    return false;
  }
  if (pa == D_BADPHYSADDR)
    return false;
  return true;
}
