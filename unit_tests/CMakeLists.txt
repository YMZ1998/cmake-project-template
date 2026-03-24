cmake_minimum_required(VERSION 3.18)

project(unit_tests)
include(${CMAKE_CURRENT_SOURCE_DIR}/src.cmake)

enable_testing()
find_package(GTest REQUIRED CONFIG)
include(GoogleTest)

add_executable(unit_tests ${unit_tests_src})

target_link_libraries(unit_tests PRIVATE
  common
  GTest::gtest
  GTest::gtest_main
)

target_include_directories(unit_tests PRIVATE
  ${SOURCE_DIR}/src
  ${SOURCE_DIR}/src/common
  ${CMAKE_CURRENT_SOURCE_DIR}
)

gtest_discover_tests(unit_tests)
