include(${SOURCE_DIR}/cmake/ProjectTargetHelpers.cmake)

set(INTEGRATION_COMMON_INCLUDE_DIRS
    ${SOURCE_DIR}/src
    ${SOURCE_DIR}/src/common
    ${SOURCE_DIR}/integration_tests
)

find_package(miniocpp REQUIRED)

function(configure_integration_target source_stem target_name)
    if(source_stem STREQUAL "test_minio")
        target_link_libraries(${target_name} PRIVATE miniocpp::miniocpp)
    endif()
endfunction()

project_add_cpp_targets(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    FOLDER "integration"
    INCLUDE_DIRS ${INTEGRATION_COMMON_INCLUDE_DIRS}
    LINK_LIBS common
    CONFIGURE_CALLBACK configure_integration_target
)
