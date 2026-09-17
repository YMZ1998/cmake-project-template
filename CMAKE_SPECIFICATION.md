# CMake 工程规范

## 1. 文档目的

本文档用于统一本项目的 CMake 配置、目录组织、编译选项、第三方依赖、测试和安装规范，确保项目能够在 Windows、Linux 以及不同编译器环境下稳定构建。

适用范围：

- 根目录 `CMakeLists.txt`
- `src/` 目录
- `examples/` 目录
- `integration_tests/` 目录
- `unit_tests/` 目录
- 第三方依赖配置
- CMake 构建、测试和安装流程

---

## 2. CMake 版本与项目声明

### 2.1 最低版本

项目最低支持 CMake 3.18：

```cmake
cmake_minimum_required(VERSION 3.18)
```

所有新增的 CMake 功能必须确认兼容 CMake 3.18。若需要提高最低版本，应同步更新本规范和构建环境文档。

### 2.2 项目声明

根目录只允许声明一次顶层项目：

```cmake
project(cmake-project-template
  LANGUAGES CXX CUDA
)
```

子目录原则上不再声明独立项目。当前 `unit_tests/CMakeLists.txt` 中的：

```cmake
project(unit_tests)
```

建议移除，避免产生不必要的项目层级和语言配置差异。

---

## 3. 推荐目录结构

```text
.
├── CMakeLists.txt
├── cmake/
│   ├── CompilerOptions.cmake
│   ├── Dependencies.cmake
│   ├── InstallRules.cmake
│   └── Platform.cmake
├── src/
│   ├── CMakeLists.txt
│   ├── common/
│   └── ...
├── examples/
│   ├── CMakeLists.txt
│   └── ...
├── integration_tests/
│   ├── CMakeLists.txt
│   └── ...
├── unit_tests/
│   ├── CMakeLists.txt
│   ├── src.cmake
│   └── ...
├── 3rdparty/
│   ├── taskflow/
│   └── nlohmann/
├── docs/
│   └── cmake-specification.md
└── output/
```

### 3.1 目录职责

| 目录 | 职责 |
|---|---|
| `src/` | 项目核心源代码和库目标 |
| `examples/` | 示例程序 |
| `unit_tests/` | 单元测试 |
| `integration_tests/` | 集成测试和环境相关测试 |
| `3rdparty/` | 随项目维护的第三方依赖 |
| `cmake/` | 可复用的 CMake 模块 |
| `docs/` | 项目文档 |
| `output/` | 构建输出目录，不应提交到 Git |

---

## 4. 根目录 CMake 规范

根目录 `CMakeLists.txt` 应按照以下顺序组织：

1. CMake 最低版本
2. CMake Policy
3. 项目声明
4. CMake 模块路径
5. 构建选项
6. 编译标准
7. 第三方依赖
8. 输出目录
9. 子目录
10. 测试和安装配置

推荐结构：

```cmake
cmake_minimum_required(VERSION 3.18)

project(cmake-project-template
  VERSION 1.0.0
  LANGUAGES CXX CUDA
)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_EXAMPLES
  "Build example executables"
  ON
)

option(BUILD_INTEGRATION_TESTS
  "Build integration and environment-dependent checks"
  ON
)

option(BUILD_UNIT_TESTS
  "Build unit tests"
  ON
)

add_subdirectory(src)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(BUILD_INTEGRATION_TESTS)
  add_subdirectory(integration_tests)
endif()

if(BUILD_UNIT_TESTS)
  add_subdirectory(unit_tests)
endif()
```

---

## 5. CMake Policy 规范

项目使用的 Policy 必须明确设置，避免不同 CMake 版本产生行为差异。

例如：

```cmake
cmake_policy(SET CMP0079 NEW)
```

新增 Policy 时应满足以下要求：

- 说明设置该 Policy 的原因；
- 确认不会破坏旧版本构建；
- 尽量在根目录统一设置；
- 不应在多个子目录中重复设置相同 Policy。

---

## 6. 编译标准

当前项目使用 C++17：

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

推荐优先使用目标级配置：

```cmake
target_compile_features(common
  PUBLIC
    cxx_std_17
)
```

### 6.1 规范要求

- 禁止依赖编译器默认 C++ 标准；
- 禁止在源文件中通过宏强制指定 C++ 标准；
- 新增目标必须明确继承或声明 C++ 标准；
- 不允许使用 GNU 扩展，除非有明确技术原因。

---

## 7. 目标管理规范

### 7.1 目标优先

CMake 配置应以 target 为中心，避免使用全局配置。

推荐：

```cmake
target_include_directories(common
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src
  PRIVATE
    ${PROJECT_SOURCE_DIR}/src/common
)
```

不推荐：

```cmake
include_directories(${PROJECT_SOURCE_DIR}/src)
```

推荐使用以下目标级命令：

- `target_include_directories`
- `target_compile_features`
- `target_compile_definitions`
- `target_compile_options`
- `target_link_libraries`
- `set_target_properties`

### 7.2 目标命名

目标名称必须具有明确含义，并避免与目录名称产生歧义。

推荐：

```cmake
add_library(project_common STATIC ...)
add_executable(project_example ...)
add_executable(project_unit_tests ...)
```

对于被多个目标复用的库，建议使用命名空间别名：

```cmake
add_library(project::common ALIAS project_common)
```

使用时：

```cmake
target_link_libraries(project_unit_tests
  PRIVATE
    project::common
)
```

### 7.3 库目标类型

除非有特殊需求，否则优先使用：

```cmake
add_library(project_common STATIC ...)
```

仅在以下场景使用共享库：

- 需要运行时动态替换库；
- 需要向外部程序提供 ABI；
- 项目明确要求动态链接。

---

## 8. 头文件和源文件管理

### 8.1 显式列出源文件

推荐显式列出源文件：

```cmake
set(COMMON_SOURCES
  common.cpp
  common.hpp
)

add_library(project_common STATIC
  ${COMMON_SOURCES}
)
```

不建议使用：

```cmake
file(GLOB_RECURSE SOURCES "*.cpp")
```

原因：

- 新增文件后 CMake 可能不会自动重新配置；
- 文件列表不直观；
- 容易意外包含生成文件或临时文件。

### 8.2 头文件可见性

头文件目录必须根据可见性选择 `PUBLIC`、`PRIVATE` 或 `INTERFACE`：

| 类型 | 含义 |
|---|---|
| `PUBLIC` | 当前目标和依赖当前目标的目标都需要 |
| `PRIVATE` | 只有当前目标需要 |
| `INTERFACE` | 当前目标本身不编译，但依赖目标需要 |

示例：

```cmake
target_include_directories(project_common
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
```

---

## 9. 第三方依赖规范

项目当前使用以下第三方依赖：

- Taskflow
- nlohmann/json
- GoogleTest
- vcpkg 管理的其他依赖

### 9.1 vcpkg 工具链

不建议在项目中固定开发者本机路径：

```cmake
set(DEFAULT_VCPKG_TOOLCHAIN "D:/vcpkg/scripts/buildsystems/vcpkg.cmake")
```

推荐通过 CMake 配置参数传入：

```bash
cmake -S . -B build ^
  -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Linux 示例：

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
```

如需提供默认路径，应允许用户通过缓存变量覆盖：

```cmake
set(VCPKG_ROOT
  ""
  CACHE PATH
  "Path to the vcpkg installation"
)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE
   AND VCPKG_ROOT
   AND EXISTS "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE
    "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE FILEPATH
    "Path to the vcpkg toolchain file"
  )
endif()
```

### 9.2 查找依赖

必须优先使用 `find_package`：

```cmake
find_package(GTest REQUIRED CONFIG)
```

依赖库应通过 target 链接：

```cmake
target_link_libraries(unit_tests
  PRIVATE
    GTest::gtest
    GTest::gtest_main
)
```

禁止手动拼接库文件路径，例如：

```cmake
target_link_libraries(app PRIVATE
  C:/some/path/lib/gtest.lib
)
```

### 9.3 第三方头文件

对于只包含头文件的依赖，应使用 `INTERFACE` 目标进行封装：

```cmake
add_library(project_taskflow INTERFACE)

target_include_directories(project_taskflow
  INTERFACE
    ${PROJECT_SOURCE_DIR}/3rdparty/taskflow
)
```

然后通过 target 使用：

```cmake
target_link_libraries(project_common
  PUBLIC
    project_taskflow
)
```

不建议在根目录使用全局配置：

```cmake
include_directories(${PROJECT_THIRDPARTY_DIR}/taskflow)
include_directories(${PROJECT_THIRDPARTY_DIR}/nlohmann)
```

---

## 10. 编译器和平台配置

### 10.1 MSVC

MSVC 相关配置应限制在 MSVC 编译器环境中：

```cmake
if(MSVC)
  target_compile_options(project_common
    PRIVATE
      /MP
      /wd4251
  )
endif()
```

不建议直接覆盖以下全局变量：

```cmake
set(CMAKE_CXX_FLAGS_RELEASE ...)
set(CMAKE_C_FLAGS_RELEASE ...)
```

原因：

- 可能覆盖用户传入的编译选项；
- 影响所有子项目；
- 容易与 CMake、vcpkg 或 IDE 配置冲突。

### 10.2 运行库配置

如果项目需要统一 MSVC 运行库，优先使用：

```cmake
set(CMAKE_MSVC_RUNTIME_LIBRARY
  "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)
```

该配置应在定义目标之前设置。

### 10.3 GCC 和 Clang

GCC 或 Clang 专用选项必须使用编译器判断：

```cmake
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(project_common
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
  )
endif()
```

禁止将 GCC 参数直接应用到 MSVC 或 CUDA 编译器。

---

## 11. CUDA 配置规范

由于项目声明了 CUDA 语言：

```cmake
project(cmake-project-template LANGUAGES CXX CUDA)
```

CUDA 配置应使用现代 CMake 目标属性，而不是只依赖旧式全局变量。

推荐：

```cmake
set_target_properties(project_cuda
  PROPERTIES
    CUDA_STANDARD 17
    CUDA_STANDARD_REQUIRED ON
)
```

如需指定 CUDA 架构：

```cmake
set_target_properties(project_cuda
  PROPERTIES
    CUDA_ARCHITECTURES native
)
```

或通过缓存变量配置：

```cmake
set(CMAKE_CUDA_ARCHITECTURES
  ""
  CACHE STRING
  "CUDA architectures to build"
)
```

不推荐仅使用：

```cmake
set(CUDA_NVCC_FLAGS_RELEASE "-O3 -Xcompiler=\"/MD\"")
set(CUDA_NVCC_FLAGS_DEBUG   "-G -g -Xcompiler=\"/MDd\"")
```

原因：

- 该方式属于较旧的全局配置方式；
- 可能无法正确应用到所有 CUDA 目标；
- 容易和 CMake 的配置生成器冲突；
- Windows 和 Linux 参数存在差异。

---

## 12. 构建类型规范

### 12.1 单配置生成器

对于 Ninja 或 Unix Makefiles：

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release
```

### 12.2 多配置生成器

对于 Visual Studio 或 Ninja Multi-Config：

```bash
cmake -S . -B build
cmake --build build --config Release
```

不建议在项目中无条件设置：

```cmake
set(CMAKE_BUILD_TYPE Release CACHE STRING "")
```

推荐：

```cmake
if(NOT CMAKE_CONFIGURATION_TYPES
   AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release
    CACHE STRING
    "Build type"
  )
endif()
```

也可以将默认构建类型交给用户或 CI 设置。

支持的构建类型：

- `Debug`
- `Release`
- `RelWithDebInfo`
- `MinSizeRel`

---

## 13. 输出目录规范

项目构建输出应集中到统一目录，例如：

```cmake
set(PROJECT_OUTPUT_DIR
  "${CMAKE_BINARY_DIR}/../output"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
  "${PROJECT_OUTPUT_DIR}"
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  "${PROJECT_OUTPUT_DIR}"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
  "${PROJECT_OUTPUT_DIR}"
)
```

对于多配置生成器，应增加配置子目录：

```cmake
foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
  string(TOUPPER "${CONFIG}" CONFIG_UPPER)

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER}
    "${PROJECT_OUTPUT_DIR}/${CONFIG}"
  )

  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER}
    "${PROJECT_OUTPUT_DIR}/${CONFIG}"
  )

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER}
    "${PROJECT_OUTPUT_DIR}/${CONFIG}"
  )
endforeach()
```

构建目录和输出目录不应提交到 Git：

```gitignore
build/
cmake-build-*/
output/
install/
```

---

## 14. 示例程序规范

示例程序必须受 `BUILD_EXAMPLES` 控制：

```cmake
option(BUILD_EXAMPLES
  "Build example executables"
  ON
)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
```

示例目标应：

- 使用项目库 target；
- 不重复配置头文件目录；
- 不依赖测试专用代码；
- 不通过绝对路径链接库文件。

示例：

```cmake
add_executable(example_basic
  example_basic.cpp
)

target_link_libraries(example_basic
  PRIVATE
    project::common
)
```

---

## 15. 单元测试规范

单元测试使用 GoogleTest 和 CTest。

推荐配置：

```cmake
include(CTest)

if(BUILD_UNIT_TESTS)
  enable_testing()

  find_package(GTest REQUIRED CONFIG)

  add_executable(project_unit_tests
    ${unit_tests_src}
  )

  target_link_libraries(project_unit_tests
    PRIVATE
      project::common
      GTest::gtest
      GTest::gtest_main
  )

  target_include_directories(project_unit_tests
    PRIVATE
      ${PROJECT_SOURCE_DIR}/src
      ${CMAKE_CURRENT_SOURCE_DIR}
  )

  include(GoogleTest)
  gtest_discover_tests(project_unit_tests)
endif()
```

### 15.1 测试要求

- 测试目标名称应具有统一前缀；
- 测试不得依赖开发者本机绝对路径；
- 测试资源文件应使用相对路径或由 CMake 显式传入；
- 单元测试和集成测试应分别控制；
- 测试失败必须返回非零退出码。

### 15.2 执行测试

```bash
ctest --test-dir build --output-on-failure
```

多配置生成器：

```bash
ctest --test-dir build -C Release --output-on-failure
```

---

## 16. 集成测试规范

集成测试可能依赖：

- GPU；
- CUDA；
- 外部服务；
- 特定硬件；
- 特定运行环境；
- 大型测试数据。

因此应通过选项控制：

```cmake
option(BUILD_INTEGRATION_TESTS
  "Build integration and environment-dependent checks"
  ON
)
```

本地快速构建时可以关闭：

```bash
cmake -S . -B build \
  -DBUILD_INTEGRATION_TESTS=OFF
```

CI 应根据运行环境决定是否启用集成测试。

---

## 17. 安装规范

安装目录应通过缓存变量配置：

```cmake
include(GNUInstallDirs)

install(
  TARGETS project_common
  EXPORT projectTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
```

不建议直接写死：

```cmake
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)
```

推荐由用户指定：

```bash
cmake -S . -B build \
  -DCMAKE_INSTALL_PREFIX=$PWD/install
```

执行安装：

```bash
cmake --install build
```

多配置生成器：

```bash
cmake --install build --config Release
```

---

## 18. 缓存变量规范

用户可配置的变量必须使用 `CACHE`：

```cmake
set(PROJECT_OUTPUT_DIR
  "${CMAKE_BINARY_DIR}/output"
  CACHE PATH
  "Directory for build outputs"
)
```

布尔开关必须使用 `option`：

```cmake
option(BUILD_EXAMPLES
  "Build example executables"
  ON
)
```

变量命名建议：

- 项目变量使用项目名称前缀；
- 全局配置使用大写；
- 路径变量使用 `_DIR` 结尾；
- 布尔变量使用 `BUILD_`、`ENABLE_` 或 `USE_` 开头。

示例：

```cmake
PROJECT_OUTPUT_DIR
PROJECT_THIRDPARTY_DIR
BUILD_UNIT_TESTS
ENABLE_CUDA
USE_CCACHE
```

---

## 19. ccache 规范

项目可以自动查找 ccache：

```cmake
find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER
    "${CCACHE_PROGRAM}"
  )
endif()
```

推荐同时支持 CUDA：

```cmake
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER
    "${CCACHE_PROGRAM}"
  )

  set(CMAKE_CUDA_COMPILER_LAUNCHER
    "${CCACHE_PROGRAM}"
  )
endif()
```

不得因为系统没有安装 ccache 而导致配置失败。

---

## 20. 构建命令规范

### 20.1 Windows + Visual Studio

```powershell
cmake -S . -B build `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DBUILD_EXAMPLES=ON `
  -DBUILD_UNIT_TESTS=ON `
  -DBUILD_INTEGRATION_TESTS=OFF

cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

### 20.2 Linux + Ninja

```bash
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_EXAMPLES=ON \
  -DBUILD_UNIT_TESTS=ON \
  -DBUILD_INTEGRATION_TESTS=OFF

cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### 20.3 使用 vcpkg

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Windows：

```powershell
cmake -S . -B build `
  -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

---

## 21. CI 构建建议

CI 至少应执行以下步骤：

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_EXAMPLES=ON \
  -DBUILD_UNIT_TESTS=ON \
  -DBUILD_INTEGRATION_TESTS=OFF

cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

CI 应覆盖：

- 至少一个 Linux 编译器；
- 至少一个 Windows 编译器；
- Debug 构建；
- Release 构建；
- 单元测试；
- 静态分析或编译器警告检查。

如项目使用 CUDA，还应在具备 CUDA 环境的 CI Runner 中执行 CUDA 构建。

---

## 22. 常见禁止事项

以下做法原则上禁止：

1. 在 CMake 中写死开发者本机路径；
2. 直接使用绝对路径链接库；
3. 大量使用全局 `include_directories`；
4. 大量修改 `CMAKE_CXX_FLAGS`；
5. 使用 `file(GLOB_RECURSE ...)` 自动收集源文件；
6. 在多个目录重复声明 `project()`；
7. 使用未声明的隐式依赖；
8. 在 CMake 中执行不可控的网络下载；
9. 将构建目录、安装目录和输出目录提交到 Git；
10. 在不同子目录中重复定义同名目标；
11. 将 GCC、MSVC、CUDA 参数混用；
12. 依赖 IDE 的默认构建配置；
13. 不区分单元测试和集成测试；
14. 将测试专用头文件目录暴露给生产目标。

---

## 23. 当前项目建议改进项

结合当前项目配置，建议优先进行以下改进：

### 23.1 移除硬编码 vcpkg 路径

当前配置包含：

```cmake
set(DEFAULT_VCPKG_TOOLCHAIN "D:/vcpkg/scripts/buildsystems/vcpkg.cmake")
```

建议改为通过 `CMAKE_TOOLCHAIN_FILE` 或 `VCPKG_ROOT` 传入��避免项目只能在特定 Windows 环境下使用。

### 23.2 减少全局头文件目录

当前配置包含：

```cmake
include_directories(${PROJECT_THIRDPARTY_DIR}/taskflow)
include_directories(${PROJECT_THIRDPARTY_DIR}/nlohmann)
```

建议改为 `INTERFACE` 依赖 target，并通过 `target_link_libraries` 传递。

### 23.3 避免覆盖全局编译参数

当前配置直接设置：

```cmake
set(CMAKE_CXX_FLAGS_${CONFIG_UPPER} ...)
set(CMAKE_C_FLAGS_${CONFIG_UPPER} ...)
```

建议使用：

- `target_compile_options`
- `target_compile_definitions`
- `CMAKE_MSVC_RUNTIME_LIBRARY`
- target 级别的链接选项

### 23.4 使用现代 CUDA 配置方式

当前配置使用：

```cmake
set(CUDA_NVCC_FLAGS_RELEASE ...)
set(CUDA_NVCC_FLAGS_DEBUG ...)
```

建议改为目标级 CUDA 属性：

```cmake
set_target_properties(project_cuda
  PROPERTIES
    CUDA_STANDARD 17
    CUDA_STANDARD_REQUIRED ON
)
```

### 23.5 移除测试子项目中的重复 `project()`

`unit_tests/CMakeLists.txt` 已经由根项目通过 `add_subdirectory` 引入，不建议再次声明：

```cmake
project(unit_tests)
```

### 23.6 改进多配置生成器输出目录

当前输出目录没有明显区分 `Debug`、`Release` 等配置，建议为 Visual Studio 等多配置生成器增加配置目录。

---

## 24. CMake 配置检查清单

提交 CMake 修改前，应确认：

- [ ] 最低 CMake 版本符合项目要求；
- [ ] 没有新增开发者本机绝对路径；
- [ ] 新增依赖通过 `find_package` 或明确的内部 target 管理；
- [ ] 新增目标使用 target 级命令；
- [ ] 头文件目录具有正确的 `PUBLIC`、`PRIVATE` 或 `INTERFACE` 属性；
- [ ] C++ 标准已明确设置；
- [ ] 编译器专用参数已正确隔离；
- [ ] CUDA 参数未错误应用到 C++ 目标；
- [ ] Debug 和 Release 均可配置；
- [ ] 单配置和多配置生成器均可使用；
- [ ] 单元测试可以被 CTest 发现；
- [ ] 构建目录和输出目录不会被提交；
- [ ] 关闭可选组件后项目仍可以正常配置；
- [ ] Linux 和 Windows 路径处理没有混用；
- [ ] CI 构建和本地构建命令保持一致。

---

## 25. 推荐验证流程

执行以下命令验证 CMake 配置：

```bash
cmake -S . -B build \
  -DBUILD_EXAMPLES=ON \
  -DBUILD_UNIT_TESTS=ON \
  -DBUILD_INTEGRATION_TESTS=OFF
```

编译：

```bash
cmake --build build --parallel
```

运行测试：

```bash
ctest --test-dir build --output-on-failure
```

清理并重新配置：

```bash
cmake --build build --target clean
cmake -S . -B build
```

验证可选模块关闭：

```bash
cmake -S . -B build-minimal \
  -DBUILD_EXAMPLES=OFF \
  -DBUILD_UNIT_TESTS=OFF \
  -DBUILD_INTEGRATION_TESTS=OFF
```

---

## 26. 版本管理建议

CMake 相关修改的提交信息建议使用以下格式：

```text
build: update CMake dependency configuration
build: add unit test target
build: improve CUDA compiler options
build: remove hardcoded local paths
ci: add Linux CMake build
test: configure GoogleTest discovery
```

涉及构建行为变化时，应在提交说明中注明：

- 支持的 CMake 版本；
- 支持的编译器；
- 是否需要更新 vcpkg 依赖；
- 是否影响默认构建选项；
- 是否影响 CUDA 架构或运行库设置。
