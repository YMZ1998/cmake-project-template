# 通用 CMake 工程规范

> 本规范适用于 C++ 项目，也适用于包含 CUDA、示例程序、单元测试和集成测试的项目。将其中的 `my_project`、版本号和目录名称替换为实际项目内容即可。

## 1. 目标

统一项目的目录结构、目标管理、依赖声明、编译选项、测试、安装和跨平台构建方式，减少对 IDE、开发者本机路径和特定编译器的隐式依赖。

核心原则：

- 使用现代 CMake，优先使用 target 级命令。
- 依赖通过 target 传递，不使用全局头文件目录和库文件路径。
- 构建目录与源码目录分离，构建结果不提交到 Git。
- 配置项通过 `option()` 或 `CACHE` 暴露给用户。
- 默认配置应能在没有可选依赖时完成最小构建。
- 不在 CMake 中执行不可控的网络下载，也不写死开发者本机路径。

## 2. 推荐目录结构

```text
.
├── CMakeLists.txt
├── cmake/
│   ├── CompilerWarnings.cmake
│   ├── Dependencies.cmake
│   └── Install.cmake
├── include/my_project/       # 对外公开的头文件
├── src/                      # 项目实现
│   └── CMakeLists.txt
├── examples/                 # 示例程序，可选
│   └── CMakeLists.txt
├── tests/
│   ├── unit/                 # 单元测试
│   ├── integration/          # 集成测试
│   └── CMakeLists.txt
├── docs/
├── 3rdparty/                 # 随项目维护的依赖（仅在必要时使用）
└── README.md
```

`build/`、`install/`、`out/` 等目录属于生成物，不应提交到仓库。

## 3. 根目录 CMakeLists.txt

根目录只声明一次 `project()`，建议使用以下顺序：最低版本、项目声明、模块路径、选项、语言标准、依赖、子目录、安装。

```cmake
cmake_minimum_required(VERSION 3.20)

project(my_project
  VERSION 1.0.0
  DESCRIPTION "A C++ project"
  LANGUAGES CXX
)

include(GNUInstallDirs)
include(CTest)

option(MY_PROJECT_BUILD_EXAMPLES "Build examples" ON)
option(MY_PROJECT_BUILD_TESTS "Build tests" ON)
option(MY_PROJECT_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(MY_PROJECT_ENABLE_INSTALL "Enable install rules" ON)

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

if(MY_PROJECT_ENABLE_INSTALL)
  include(cmake/Install.cmake)
endif()
```

不要无条件覆盖用户传入的 `CMAKE_BUILD_TYPE`、`CMAKE_INSTALL_PREFIX`、工具链文件或编译器标志。单配置生成器的构建类型应由命令行指定：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

多配置生成器应在构建和测试时指定配置：

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## 4. Target 规范

### 4.1 目标优先

禁止用全局命令污染所有目标：

```cmake
# 不推荐
include_directories(include)
add_compile_options(-Wall)
link_directories(/some/path)
```

使用目标级命令：

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

### 4.2 可见性

- `PUBLIC`：当前目标和使用当前目标的目标都需要。
- `PRIVATE`：仅当前目标需要。
- `INTERFACE`：当前目标不编译，但使用者需要。

每个库都应提供命名空间别名，例如 `my_project::core`，其他目标只链接别名，不依赖内部目标名。

### 4.3 源文件

优先显式列出源文件：

```cmake
set(MY_PROJECT_SOURCES
  core.cpp
  parser.cpp
)
add_library(my_project_core ${MY_PROJECT_SOURCES})
```

除非项目明确接受重新配置风险，否则不要使用 `file(GLOB_RECURSE ...)` 自动收集源文件。新增源文件应显式修改 CMake 文件。

## 5. 编译器选项

警告选项必须限定到目标和对应编译器：

```cmake
if(MY_PROJECT_ENABLE_WARNINGS)
  if(MSVC)
    target_compile_options(my_project_core PRIVATE /W4 /permissive-)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(my_project_core PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endif()
```

禁止直接重写 `CMAKE_CXX_FLAGS`、`CMAKE_CXX_FLAGS_RELEASE` 等全局变量。MSVC 专用选项不能传给 GCC、Clang 或 CUDA 编译器。需要统一 MSVC 运行库时，优先在定义目标前设置：

```cmake
set(CMAKE_MSVC_RUNTIME_LIBRARY
  "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
```

可选使用 ccache，但系统未安装时不得使配置失败：

```cmake
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()
```

## 6. 第三方依赖

优先使用包管理器和 `find_package()`，通过 imported target 链接：

```cmake
find_package(fmt CONFIG REQUIRED)
target_link_libraries(my_project_core PRIVATE fmt::fmt)
```

不要直接链接绝对路径：

```cmake
# 错误示例
# target_link_libraries(app PRIVATE C:/libs/fmt.lib)
```

头文件依赖也应封装成 `INTERFACE` target：

```cmake
add_library(my_project_dependency INTERFACE)
target_include_directories(my_project_dependency INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/3rdparty/example/include>
)
target_link_libraries(my_project_core PUBLIC my_project_dependency)
```

vcpkg、Conan 等工具链由配置命令传入，不在项目中写死路径：

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
```

## 7. 测试

测试统一由 CTest 管理，单元测试和需要外部服务、GPU、网络或大型数据的集成测试分开控制：

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

测试资源不得依赖开发者本机绝对路径。执行测试：

```bash
ctest --test-dir build --output-on-failure
```

集成测试可通过 `MY_PROJECT_BUILD_INTEGRATION_TESTS` 关闭，CI 根据运行环境决定是否启用。

## 8. CUDA 项目

只有确实需要 CUDA 的项目才在 `project()` 中声明 `CUDA`：

```cmake
project(my_project LANGUAGES CXX CUDA)

add_library(my_project_cuda cuda_kernels.cu)
set_target_properties(my_project_cuda PROPERTIES
  CUDA_STANDARD 17
  CUDA_STANDARD_REQUIRED ON
)
```

优先使用 `CUDA_ARCHITECTURES` 和目标属性，不使用旧式全局 `CUDA_NVCC_FLAGS`。CUDA 是可选功能时，应使用选项并通过 `CheckLanguage` 或独立子目录隔离配置。

## 9. 安装与导出

安装路径由用户控制，不在项目中强制设置 `CMAKE_INSTALL_PREFIX`：

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

install(EXPORT my_projectTargets
  FILE my_projectTargets.cmake
  NAMESPACE my_project::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/my_project
)
```

构建和安装：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build --prefix "$PWD/install"
```

## 10. 代码检查与 CI

CI 至少应执行：

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DMY_PROJECT_BUILD_EXAMPLES=ON \
  -DMY_PROJECT_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

建议覆盖 Linux、Windows、Debug、Release、至少一种 GCC/Clang 和一种 MSVC，并加入 clang-format、clang-tidy 或编译器警告检查。CI 中应使用干净的构建目录，避免复用开发者缓存。

## 11. Git 忽略项

至少忽略以下目录：

```gitignore
/build/
/cmake-build-*/
/out/
/output/
/install/
compile_commands.json
```

## 12. 禁止事项清单

1. 写死开发者本机路径、编译器路径或 vcpkg 路径。
2. 直接链接绝对路径库文件。
3. 在根目录大量使用 `include_directories()`、`link_directories()`。
4. 无条件修改全局编译标志或构建类型。
5. 在子目录重复调用 `project()`。
6. 使用未声明的隐式依赖。
7. 混用 GCC、MSVC、CUDA 专用参数。
8. 将构建、安装和生成输出提交到 Git。
9. 让集成测试阻塞不需要外部环境的最小构建。
10. 在配置阶段执行没有版本、校验和或离线策略的网络下载。

## 13. 推荐命令速查

```bash
# 配置
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# 编译
cmake --build build --parallel

# 测试
ctest --test-dir build --output-on-failure

# 安装
cmake --install build --prefix "$PWD/install"

# 清理（推荐直接删除构建目录）
rm -rf build install
```
