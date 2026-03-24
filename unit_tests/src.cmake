set(dir "${CMAKE_CURRENT_SOURCE_DIR}")

message("unit_tests dir: ${dir}")
file(GLOB unit_tests_root_src CONFIGURE_DEPENDS
    "${dir}/*.cpp"
)

file(GLOB unit_tests_case_src CONFIGURE_DEPENDS
    "${dir}/src/*.cpp"
)

set(unit_tests_src
    ${unit_tests_root_src}
    ${unit_tests_case_src}
)

message("unit_tests_src: ${unit_tests_src}")
