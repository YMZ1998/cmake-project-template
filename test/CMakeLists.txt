find_package(spdlog REQUIRED)
find_package(benchmark REQUIRED)
find_package(miniocpp REQUIRED)

function(add_my_test name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE common spdlog::spdlog benchmark::benchmark  miniocpp::miniocpp)
    target_include_directories(${name} PRIVATE 
        ${SOURCE_DIR}/src
        ${SOURCE_DIR}/src/common
        ${SOURCE_DIR}/test
    )
endfunction()

file(GLOB TEST_SRCS "*.cpp")

foreach(src_file IN LISTS TEST_SRCS)
    get_filename_component(name ${src_file} NAME_WE)
    add_my_test(${name} ${src_file})
    set_target_properties(${name} PROPERTIES FOLDER "test")
    message(STATUS "Added test target: ${name}")
endforeach()

source_group("Source Files" FILES ${TEST_SRCS})

