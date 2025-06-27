set(dir "${PROJECT_SOURCE_DIR}")
message("common dir: ${dir}")

file(GLOB common_header
    "${dir}/*.h"
    "${dir}/*.hpp"
	"${dir}/*.cuh"
)
file(GLOB common_src
    "${dir}/*.cpp"
    "${dir}/*.cu"
)

message("common_header: ${common_header}")