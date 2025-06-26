cmake_minimum_required(VERSION 3.18)

project(unit_tests)
include(${CMAKE_CURRENT_SOURCE_DIR}/src.cmake)

set(GTest_DIR "D:/vcpkg/installed/x64-windows/share/gtest")
enable_testing()
find_package(GTest REQUIRED CONFIG)
include(GoogleTest)

include_directories(${DIVISION_HEADERS_DIR})

add_executable(unit_tests ${unit_tests_src})

target_link_libraries(unit_tests PRIVATE
  GTest::gtest
  GTest::gtest_main
)
