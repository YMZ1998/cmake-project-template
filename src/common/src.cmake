set(dir "${CMAKE_CURRENT_SOURCE_DIR}")
message("common dir: ${dir}")

file(GLOB common_header CONFIGURE_DEPENDS
    "${dir}/*.h"
    "${dir}/*.hpp"
    "${dir}/*.cuh"
)

file(GLOB common_src CONFIGURE_DEPENDS
    "${dir}/*.cpp"
    "${dir}/*.cu"
)

message("common_header: ${common_header}")
