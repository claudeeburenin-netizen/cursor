# EntryState tests

Unit tests for the `EntryState` struct, written with
[GoogleTest](https://github.com/google/googletest).

## Layout

- `entry_state.h` — the struct under test.
- `entry_state_test.cc` — the GoogleTest test suite.
- `CMakeLists.txt` — build script that fetches GoogleTest via `FetchContent`.

## The struct under test

```cpp
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
```

## What is covered

The test suite (`EntryStateTest`) covers:

- Default-constructed field values.
- The first `add` call stores `new_version` regardless of `is_reusable_size`
  (including `new_version == 0`).
- `count` is incremented on every call.
- `version` is *not* overwritten by subsequent calls.
- The `multiple_versions` flag:
  - stays `false` when versions match;
  - stays `false` when versions differ but `is_reusable_size` is `true`;
  - becomes `true` only when versions differ *and* `is_reusable_size` is
    `false`;
  - is sticky (a later "clean" call cannot clear it).
- A full truth table for the predicate inside `add`.
- Edge values such as `std::numeric_limits<uint32_t>::max()`.
- Independence between separate `EntryState` instances.
- A stress loop that performs many adds and verifies `count` and `version`.

## Building and running

Requires CMake ≥ 3.14, a C++17 compiler, and network access (to fetch
GoogleTest the first time).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
# or run the binary directly:
./build/entry_state_test
```

If your default `c++` driver is misconfigured, force GCC explicitly:

```bash
CXX=g++ CC=gcc cmake -S . -B build
```
