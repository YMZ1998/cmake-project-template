set(EXAMPLE_COMMON_INCLUDE_DIRS
    ${SOURCE_DIR}/src
    ${SOURCE_DIR}/src/common
    ${SOURCE_DIR}/examples
)

function(add_project_example name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE common)
    target_include_directories(${name} PRIVATE ${EXAMPLE_COMMON_INCLUDE_DIRS})
    set_target_properties(${name} PROPERTIES FOLDER "examples")
endfunction()

find_package(spdlog REQUIRED)
find_package(benchmark REQUIRED)

file(GLOB EXAMPLE_SRCS CONFIGURE_DEPENDS "*.cpp")

foreach(example_src IN LISTS EXAMPLE_SRCS)
    get_filename_component(example_name ${example_src} NAME_WE)

    if(example_name STREQUAL "test")
        set(example_target "example_benchmark")
    else()
        set(example_target ${example_name})
    endif()

    add_project_example(${example_target} ${example_src})

    if(example_name STREQUAL "test")
        target_link_libraries(${example_target} PRIVATE spdlog::spdlog benchmark::benchmark)
    endif()
endforeach()
