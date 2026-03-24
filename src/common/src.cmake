set(dir "${CMAKE_CURRENT_SOURCE_DIR}")
message("common dir: ${dir}")

set(common_header
    "${dir}/argparse.hpp"
    "${dir}/common.h"
    "${dir}/logger.hpp"
    "${dir}/recycle_manager.h"
    "${dir}/test_cuda.h"
    "${dir}/wlog.hpp"
)

set(common_src
    "${dir}/common.cpp"
    "${dir}/recycle_manager.cpp"
    "${dir}/test_cuda.cu"
)

message("common_header: ${common_header}")
