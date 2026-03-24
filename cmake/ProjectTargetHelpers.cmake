include(CMakeParseArguments)

function(project_resolve_target_name source_stem out_var)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs TARGET_NAME_OVERRIDES)
  cmake_parse_arguments(PRTN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(target_name "${source_stem}")

  foreach(name_override IN LISTS PRTN_TARGET_NAME_OVERRIDES)
    string(REPLACE "=" ";" override_parts "${name_override}")
    list(LENGTH override_parts override_length)
    if(NOT override_length EQUAL 2)
      message(FATAL_ERROR "Invalid TARGET_NAME_OVERRIDES entry: ${name_override}")
    endif()

    list(GET override_parts 0 override_source_stem)
    list(GET override_parts 1 override_target_name)

    if(override_source_stem STREQUAL source_stem)
      set(target_name "${override_target_name}")
      break()
    endif()
  endforeach()

  set(${out_var} "${target_name}" PARENT_SCOPE)
endfunction()

function(project_add_cpp_targets)
  set(options)
  set(oneValueArgs DIRECTORY FOLDER CONFIGURE_CALLBACK)
  set(multiValueArgs INCLUDE_DIRS LINK_LIBS TARGET_NAME_OVERRIDES)
  cmake_parse_arguments(PACT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT PACT_DIRECTORY)
    message(FATAL_ERROR "project_add_cpp_targets requires DIRECTORY")
  endif()

  if(NOT PACT_FOLDER)
    message(FATAL_ERROR "project_add_cpp_targets requires FOLDER")
  endif()

  file(GLOB target_sources CONFIGURE_DEPENDS
      "${PACT_DIRECTORY}/*.cpp"
  )

  foreach(target_source IN LISTS target_sources)
    get_filename_component(source_stem "${target_source}" NAME_WE)

    project_resolve_target_name(
        "${source_stem}"
        target_name
        TARGET_NAME_OVERRIDES ${PACT_TARGET_NAME_OVERRIDES}
    )

    add_executable(${target_name} "${target_source}")

    if(PACT_LINK_LIBS)
      target_link_libraries(${target_name} PRIVATE ${PACT_LINK_LIBS})
    endif()

    if(PACT_INCLUDE_DIRS)
      target_include_directories(${target_name} PRIVATE ${PACT_INCLUDE_DIRS})
    endif()

    set_target_properties(${target_name} PROPERTIES FOLDER "${PACT_FOLDER}")

    if(PACT_CONFIGURE_CALLBACK)
      cmake_language(CALL ${PACT_CONFIGURE_CALLBACK} "${source_stem}" "${target_name}")
    endif()
  endforeach()
endfunction()
