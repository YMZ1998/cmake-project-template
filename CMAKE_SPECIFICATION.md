# CMake 简版规范（团队版）

适用范围：C++ / C++ + CUDA 项目。目标是统一工程结构、依赖方式、编译规则和测试流程，减少“写着能跑、别人不敢改”的配置问题。

## 1. 基本原则

- 使用现代 CMake，优先使用 `target_*` 命令，不用全局 `include_directories()` 和 `link_directories()`。
- 根目录只保留一个 `project()`。
- 依赖通过 `find_package()` + `target_link_libraries()` 引入。
- 构建目录和源码目录分离，构建产物不提交到 Git。
- 用户配置项通过 `option()` 或 `CACHE` 暴露。
- 不写死开发者本机路径，不在 CMake 中做网络下载。

## 2. 推荐目录结构

```text
.
├── CMakeLists.txt
├── cmake/
│   ├── CompilerWarnings.cmake
│   ├── Dependencies.cmake
│   └── Install.cmake
├── include/
│   └── my_project/
├── src/
│   ├── CMakeLists.txt
│   └── ...
├── examples/
│   └── CMakeLists.txt
├── tests/
│   ├── unit/
│   ├── integration/
│   └── CMakeLists.txt
├── 3rdparty/
├── docs/
├── README.md
├── build/
├── install/
└── out/
```

说明：`build/`、`install/`、`out/` 属于构建产物，通常写进 `.gitignore`。

## 3. 根目录 CMakeLists.txt 模板

```cmake
cmake_minimum_required(VERSION 3.20)

project(my_project
  VERSION 1.0.0
  DESCRIPTION "My Project"
  LANGUAGES CXX
)

include(GNUInstallDirs)
include(CTest)

option(MY_PROJECT_BUILD_EXAMPLES "Build example programs" ON)
option(MY_PROJECT_BUILD_TESTS "Build tests" ON)
option(MY_PROJECT_ENABLE_WARNINGS "Enable compiler warnings" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)

if(MY_PROJECT_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(BUILD_TESTING AND MY_PROJECT_BUILD_TESTS)
  add_subdirectory(tests)
endif()
```

补充：

- 单配置构建器：`-DCMAKE_BUILD_TYPE=Release`
- 多配置构建器：`cmake --build build --config Release`
- 不要无条件覆盖 `CMAKE_BUILD_TYPE`、`CMAKE_INSTALL_PREFIX` 或工具链文件

## 4. Target 规范

### 4.1 正确方式

```cmake
add_library(my_project_core
  core.cpp
  core.hpp
)

add_library(my_project::core ALIAS my_project_core)

target_include_directories(my_project_core
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(my_project_core PUBLIC cxx_std_17)
```

### 4.2 规范要求

- 公共头文件使用 `PUBLIC`
- 仅当前目标使用的头文件使用 `PRIVATE`
- 第三方只读依赖可用 `INTERFACE` 目标包一层
- 目标名称请带语义，如 `my_project_core`、`my_project_example`
- 不要重复定义同名目标

## 5. 编译选项和警告

```cmake
if(MY_PROJECT_ENABLE_WARNINGS)
  if(MSVC)
    target_compile_options(my_project_core PRIVATE /W4 /permissive-)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(my_project_core PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endif()
```

禁止：

- 直接修改 `CMAKE_CXX_FLAGS`
- 在不同平台混用 GCC / MSVC / CUDA 选项
- 让项目依赖 IDE 默认配置

对 MSVC 统一运行库时，优先在定义目标前使用：

```cmake
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
```

## 6. 第三方依赖

优先用 `find_package()` + imported target：

```cmake
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

target_link_libraries(my_project_core PRIVATE fmt::fmt)
target_link_libraries(my_project_unit_tests PRIVATE my_project::core GTest::gtest_main)
```

禁止：

- 直接链接 `C:/xxx/lib/*.lib` 这样的绝对路径
- 在工程里写死开发者本机 vcpkg 路径
- 直接 `include_directories(${PROJECT_SOURCE_DIR}/3rdparty/... )`

vcpkg / Conan 等工具链由命令行注入：

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
```

## 7. 测试规范

- 单元测试和集成测试分开控制
- 单元测试走 CTest + GTest
- 资源文件必须使用相对路径或显式传入
- 测试失败必须返回非零退出码

```cmake
find_package(GTest CONFIG REQUIRED)

add_executable(my_project_unit_tests
  unit/core_test.cpp
)

target_link_libraries(my_project_unit_tests PRIVATE
  my_project::core
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(my_project_unit_tests)
```

执行：

```bash
ctest --test-dir build --output-on-failure
```

集成测试建议通过开关控制：

```cmake
option(MY_PROJECT_BUILD_INTEGRATION_TESTS "Build integration tests" OFF)
```

## 8. CUDA 规范

仅在项目确实需要 CUDA 时声明：

```cmake
project(my_project LANGUAGES CXX CUDA)

add_library(my_project_cuda cuda_kernels.cu)
set_target_properties(my_project_cuda PROPERTIES
  CUDA_STANDARD 17
  CUDA_STANDARD_REQUIRED ON
)
```

建议：

- 使用 `CUDA_ARCHITECTURES` 和目标属性
- 不要使用旧式全局 `CUDA_NVCC_FLAGS`
- 若 CUDA 可选，放在独立子目录/选项中控制

## 9. 安装与导出

```cmake
include(GNUInstallDirs)

install(TARGETS my_project_core
  EXPORT my_projectTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
```

执行：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build --prefix "$PWD/install"
```

## 10. CI 规范

CI 至少执行：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

建议覆盖：

- Linux + GCC/Clang
- Windows + MSVC
- Debug/Release
- 单元测试
- 编译器警告或静态检查

## 11. Git 忽略建议

```gitignore
/build/
/cmake-build-*/
/out/
/output/
/install/
compile_commands.json
```

## 12. 禁止事项清单

1. 写死开发者本机路径
2. 直接绝对路径链接库
3. 大量使用全局 `include_directories()`
4. 修改全局 `CMAKE_CXX_FLAGS`
5. 在子目录重复声明 `project()`
6. 隐式依赖未声明的库
7. 混用 GCC / MSVC / CUDA 参数
8. 将构建产物提交到 Git
9. 集成测试阻塞普通开发构建

## 13. 最小可用模板

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_project LANGUAGES CXX)

include(GNUInstallDirs)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MY_PROJECT_BUILD_EXAMPLES "Build examples" ON)
option(MY_PROJECT_BUILD_TESTS "Build tests" ON)

add_subdirectory(src)

if(MY_PROJECT_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(BUILD_TESTING AND MY_PROJECT_BUILD_TESTS)
  add_subdirectory(tests)
endif()
```

这版规范的重点是：简单、统一、可执行、适合团队协作。对新项目而言，直接按这套规则落地即可，后续再按需要扩展。
