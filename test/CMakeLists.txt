include_directories("${PROJECT_SOURCE_DIR}/test")
include_directories("${PROJECT_SOURCE_DIR}/src")

project(test)

add_compile_options("/std:c++17")

add_definitions(-D_BIND_TO_CURRENT_VCLIBS_VERSION=1)
add_definitions(-D_UNICODE -DUNICODE)

add_executable(test test.cpp)

target_link_libraries(test
)

