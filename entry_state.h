#ifndef ENTRY_STATE_H_
#define ENTRY_STATE_H_

#include <cstdint>

struct EntryState {
  uint32_t version = 0;
  int count = 0;
  bool multiple_versions = false;

  void add(uint32_t new_version, bool is_reusable_size) {
    if (count == 0) {
      version = new_version;
    }
    multiple_versions |= !is_reusable_size && new_version != version;
    count++;
  }
};

#endif  // ENTRY_STATE_H_
