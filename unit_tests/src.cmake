set(dir "${CMAKE_SOURCE_DIR}/unit_tests")

message("dir: ${dir}")
file(GLOB unit_tests_src
    "${dir}/*.cpp"
    "${dir}/src/*.cpp"
)

message("unit_tests_src: ${unit_tests_src}")

