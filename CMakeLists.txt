cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0079 NEW)

set(DEFAULT_VCPKG_TOOLCHAIN "D:/vcpkg/scripts/buildsystems/vcpkg.cmake")
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "${DEFAULT_VCPKG_TOOLCHAIN}")
  set(CMAKE_TOOLCHAIN_FILE "${DEFAULT_VCPKG_TOOLCHAIN}" CACHE STRING "Path to the vcpkg toolchain file")
endif()

project(cmake-project-template LANGUAGES CXX CUDA)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()
set(CMAKE_BUILD_TYPE Release CACHE STRING "")

set(PROJECT_OUTPUT_DIR "${CMAKE_BINARY_DIR}/../output")
foreach(type RUNTIME LIBRARY ARCHIVE)
  set(CMAKE_${type}_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR})
endforeach()

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)
set(SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(PROJECT_THIRDPARTY_DIR ${PROJECT_SOURCE_DIR}/3rdparty)
message(STATUS "3rdparty: ${PROJECT_THIRDPARTY_DIR}")

include_directories(${PROJECT_THIRDPARTY_DIR}/taskflow)
include_directories(${PROJECT_THIRDPARTY_DIR}/nlohmann)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
  add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:/MP>
    $<$<COMPILE_LANGUAGE:CXX>:/wd4251>
  )
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

option(BUILD_EXAMPLES "Build example executables" ON)
option(BUILD_INTEGRATION_TESTS "Build integration and environment-dependent checks" OFF)
if(BUILD_EXAMPLES OR BUILD_INTEGRATION_TESTS)
  add_subdirectory(test)
endif()

option(BUILD_UNIT_TESTS "Build unit tests" ON)
if(BUILD_UNIT_TESTS)
  add_subdirectory(unit_tests)
endif()
