cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0079 NEW)

set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

project(cmake-project-template LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
set(CMAKE_BUILD_TYPE Release CACHE STRING "")

set(PROJECT_OUTPUT_DIR "${CMAKE_BINARY_DIR}/../output")
foreach(type RUNTIME LIBRARY ARCHIVE)
  set(CMAKE_${type}_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR})
endforeach()

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)
set(SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(3rdparty ${PROJECT_SOURCE_DIR}/3rdparty)
message(STATUS "3rdparty: ${3rdparty}")

include_directories(${3rdparty}/taskflow)
include_directories(${3rdparty}/nlohmann)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
  add_compile_options(/MP /wd4251)

  foreach(config DEBUG RELEASE RELWITHDEBINFO)
    string(TOUPPER "${config}" CONFIG_UPPER)
    if(CONFIG_UPPER STREQUAL "DEBUG")
      set(RUNTIME_FLAG "/MDd")
      set(ITERATOR_DEBUG "/D_ITERATOR_DEBUG_LEVEL=2 /Zi")
      set(OPT_FLAG "")
      set(LINKER_DEBUG "/DEBUG")
    elseif(CONFIG_UPPER STREQUAL "RELWITHDEBINFO")
      set(RUNTIME_FLAG "/MD")
      set(ITERATOR_DEBUG "/D_ITERATOR_DEBUG_LEVEL=0 /Zi")
      set(OPT_FLAG "/O2")
      set(LINKER_DEBUG "/DEBUG")
    else()
      set(RUNTIME_FLAG "/MD")
      set(ITERATOR_DEBUG "/D_ITERATOR_DEBUG_LEVEL=0 /Zi")
      set(OPT_FLAG "/O2")
      set(LINKER_DEBUG "/DEBUG")
    endif()

    set(CMAKE_CXX_FLAGS_${CONFIG_UPPER} "${OPT_FLAG} ${RUNTIME_FLAG} ${ITERATOR_DEBUG}")
    set(CMAKE_C_FLAGS_${CONFIG_UPPER}   "${OPT_FLAG} ${RUNTIME_FLAG} ${ITERATOR_DEBUG}")
    set(CMAKE_SHARED_LINKER_FLAGS_${CONFIG_UPPER} "${LINKER_DEBUG}")
    set(CMAKE_EXE_LINKER_FLAGS_${CONFIG_UPPER} "${LINKER_DEBUG}")
  endforeach()
endif()

set(CUDA_NVCC_FLAGS_RELEASE "-O3 -Xcompiler=\"/MD\"")
set(CUDA_NVCC_FLAGS_DEBUG   "-G -g -Xcompiler=\"/MDd\"")

add_subdirectory(src)

option(build_test "Build test" ON)
if(build_test)
  add_subdirectory(test)
endif()

option(build_unit_tests "Build unit_tests" ON)
if(build_unit_tests)
  add_subdirectory(unit_tests)
endif()
