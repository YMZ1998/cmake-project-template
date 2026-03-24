set(INTEGRATION_COMMON_INCLUDE_DIRS
    ${SOURCE_DIR}/src
    ${SOURCE_DIR}/src/common
    ${SOURCE_DIR}/integration_tests
)

function(add_project_integration_check name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE common)
    target_include_directories(${name} PRIVATE ${INTEGRATION_COMMON_INCLUDE_DIRS})
    set_target_properties(${name} PROPERTIES FOLDER "integration")
endfunction()

find_package(miniocpp REQUIRED)

file(GLOB INTEGRATION_SRCS CONFIGURE_DEPENDS "*.cpp")

foreach(integration_src IN LISTS INTEGRATION_SRCS)
    get_filename_component(integration_name ${integration_src} NAME_WE)
    add_project_integration_check(${integration_name} ${integration_src})

    if(integration_name STREQUAL "test_minio")
        target_link_libraries(${integration_name} PRIVATE miniocpp::miniocpp)
    endif()
endforeach()
