#ifndef QUERY_PDB_SERVER_HANDLE_GUARD_H
#define QUERY_PDB_SERVER_HANDLE_GUARD_H

#include "ExampleMemoryMappedFile.h"

class handle_guard {
public:
  handle_guard() = default;

  explicit handle_guard(MemoryMappedFile::Handle handle) : handle_(handle) {}

  MemoryMappedFile::Handle &get() { return handle_; }

  const MemoryMappedFile::Handle &get() const { return handle_; }

  bool valid() const { return handle_.baseAddress != nullptr; }

  ~handle_guard() {
    if (valid()) {
      MemoryMappedFile::Close(handle_);
    }
  }

  handle_guard(const handle_guard &) = delete;

  handle_guard &operator=(const handle_guard &) = delete;

  handle_guard(handle_guard &&other) = delete;

  handle_guard &operator=(handle_guard &&other) = delete;

private:
  MemoryMappedFile::Handle handle_;
};

#endif // QUERY_PDB_SERVER_HANDLE_GUARD_H
