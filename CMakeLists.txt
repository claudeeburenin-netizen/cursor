cmake_minimum_required(VERSION 3.14)
project(entry_state_tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: prevent overriding the parent project's compiler/linker settings.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(entry_state_test entry_state_test.cc)
target_include_directories(entry_state_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(entry_state_test PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(entry_state_test)
