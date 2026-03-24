set(TEST_COMMON_INCLUDE_DIRS
    ${SOURCE_DIR}/src
    ${SOURCE_DIR}/src/common
    ${SOURCE_DIR}/test
)

function(add_project_example name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE common)
    target_include_directories(${name} PRIVATE ${TEST_COMMON_INCLUDE_DIRS})
    set_target_properties(${name} PROPERTIES FOLDER "examples")
endfunction()

function(add_project_integration_check name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE common)
    target_include_directories(${name} PRIVATE ${TEST_COMMON_INCLUDE_DIRS})
    set_target_properties(${name} PROPERTIES FOLDER "integration")
endfunction()

if(BUILD_EXAMPLES)
    find_package(spdlog REQUIRED)
    find_package(benchmark REQUIRED)

    add_project_example(example_benchmark test.cpp)
    target_link_libraries(example_benchmark PRIVATE spdlog::spdlog benchmark::benchmark)

    foreach(example_src IN ITEMS test2.cpp test3.cpp test4.cpp test_recycle_manager.cpp)
        get_filename_component(example_name ${example_src} NAME_WE)
        add_project_example(${example_name} ${example_src})
    endforeach()
endif()

if(BUILD_INTEGRATION_TESTS)
    add_project_integration_check(test_cuda test_cuda.cpp)

    find_package(miniocpp REQUIRED)
    add_project_integration_check(test_minio test_minio.cpp)
    target_link_libraries(test_minio PRIVATE miniocpp::miniocpp)
endif()
