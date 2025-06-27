cmake_minimum_required(VERSION 3.18)

set(LIBRARY_NAME "common")
project(${LIBRARY_NAME})

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

include(${CMAKE_CURRENT_SOURCE_DIR}/src.cmake)
set(SOURCE_FILES ${common_header} ${common_src})

#add_library(${LIBRARY_NAME} SHARED ${SOURCE_FILES})
add_library(${LIBRARY_NAME} STATIC ${SOURCE_FILES})
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${LIBRARY_NAME} PRIVATE 
 spdlog::spdlog
 fmt::fmt
)

install(FILES ${common_header} DESTINATION common/include)

install(TARGETS ${LIBRARY_NAME}
    RUNTIME DESTINATION common/bin/$<CONFIG>
    LIBRARY DESTINATION common/lib/$<CONFIG>
    ARCHIVE DESTINATION common/lib/$<CONFIG>
)
