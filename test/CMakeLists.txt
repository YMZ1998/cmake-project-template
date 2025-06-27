project(test)

find_package(spdlog REQUIRED)

add_executable(test test.cpp)

target_link_libraries(test PRIVATE 
    common
    spdlog::spdlog
)
get_target_property(spdlog_include_dirs spdlog::spdlog INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "spdlog include dirs: ${spdlog_include_dirs}")

target_include_directories(test PRIVATE 
    ${SOURCE_DIR}/src
    ${SOURCE_DIR}/src/common
    ${SOURCE_DIR}/test
)

target_compile_features(test PRIVATE cxx_std_17)

target_compile_definitions(test PRIVATE
    _UNICODE
    UNICODE
    _BIND_TO_CURRENT_VCLIBS_VERSION=1
)
