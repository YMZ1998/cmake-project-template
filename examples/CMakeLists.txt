include(${SOURCE_DIR}/cmake/ProjectTargetHelpers.cmake)

set(EXAMPLE_COMMON_INCLUDE_DIRS
    ${SOURCE_DIR}/src
    ${SOURCE_DIR}/src/common
    ${SOURCE_DIR}/examples
)

find_package(spdlog REQUIRED)
find_package(benchmark REQUIRED)

function(configure_example_target source_stem target_name)
    if(source_stem STREQUAL "test")
        target_link_libraries(${target_name} PRIVATE spdlog::spdlog benchmark::benchmark)
    endif()
endfunction()

project_add_cpp_targets(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    FOLDER "examples"
    INCLUDE_DIRS ${EXAMPLE_COMMON_INCLUDE_DIRS}
    LINK_LIBS common
    TARGET_NAME_OVERRIDES "test=example_benchmark"
    CONFIGURE_CALLBACK configure_example_target
)
