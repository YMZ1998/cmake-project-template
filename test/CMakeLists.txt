find_package(spdlog REQUIRED)
find_package(benchmark REQUIRED)

function(add_my_test name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE common spdlog::spdlog benchmark::benchmark)
    target_include_directories(${name} PRIVATE 
        ${SOURCE_DIR}/src
        ${SOURCE_DIR}/src/common
        ${SOURCE_DIR}/test
    )
    target_compile_features(${name} PRIVATE cxx_std_17)
    target_compile_definitions(${name} PRIVATE
        _UNICODE
        UNICODE
        _BIND_TO_CURRENT_VCLIBS_VERSION=1
    )
    set_target_properties(${name}  PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${name}>"
)
endfunction()

add_my_test(test  test.cpp)
add_my_test(test2 test2.cpp)
add_my_test(test3 test3.cpp)
