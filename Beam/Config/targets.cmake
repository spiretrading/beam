add_library(BeamRoutines INTERFACE)
if(UNIX)
  find_package(Threads REQUIRED)
  target_link_libraries(BeamRoutines INTERFACE
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    Threads::Threads ${CMAKE_DL_LIBS})
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(BeamRoutines INTERFACE rt)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    target_link_libraries(BeamRoutines INTERFACE rt socket nsl)
  endif()
endif()

function(beam_configure_target target)
  cmake_parse_arguments(PARSE_ARGV 1 arguments "" "" "DEPENDENCIES")
  if(arguments_UNPARSED_ARGUMENTS OR arguments_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "Invalid beam_configure_target arguments.")
  endif()
  if(MSVC)
    set_target_properties(${target} PROPERTIES
      VS_GLOBAL_UseMultiToolTask true
      VS_GLOBAL_EnforceProcessCountAcrossBuilds true
      VS_GLOBAL_CL_MPCount "$([System.Environment]::ProcessorCount)")
    target_compile_options(
      ${target} PRIVATE /bigobj /external:anglebrackets /external:W0
      $<$<CONFIG:Release>:/GL> /MP /WX /Zc:__cplusplus /Zc:preprocessor)
    target_compile_definitions(
      ${target} PRIVATE _CRT_SECURE_NO_DEPRECATE NOMINMAX
      _SCL_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
    target_link_options(${target} PRIVATE $<$<CONFIG:Release>:/LTCG>)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target} PRIVATE -g $<$<CONFIG:Release>:-DNDEBUG>)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
      target_compile_options(${target} PRIVATE -fsized-deallocation)
    endif()
  endif()
  if(CYGWIN)
    target_compile_definitions(${target} PRIVATE __USE_W32_SOCKETS)
  endif()
  foreach(dependency IN LISTS arguments_DEPENDENCIES)
    if(DEFINED ${dependency}_INCLUDE_PATH)
      target_include_directories(
        ${target} SYSTEM PRIVATE ${${dependency}_INCLUDE_PATH})
    endif()
    if(DEFINED ${dependency}_LIBRARY_DEBUG_PATH OR
        DEFINED ${dependency}_LIBRARY_OPTIMIZED_PATH)
      if(NOT DEFINED ${dependency}_LIBRARY_DEBUG_PATH OR
          NOT DEFINED ${dependency}_LIBRARY_OPTIMIZED_PATH)
        message(FATAL_ERROR "Incomplete library paths for ${dependency}.")
      endif()
      target_link_libraries(
        ${target} PUBLIC debug ${${dependency}_LIBRARY_DEBUG_PATH}
        optimized ${${dependency}_LIBRARY_OPTIMIZED_PATH})
    elseif(NOT DEFINED ${dependency}_INCLUDE_PATH)
      message(FATAL_ERROR "Unknown dependency: ${dependency}")
    endif()
  endforeach()
endfunction()

function(beam_install_target target directory)
  install(TARGETS ${target} DESTINATION "${directory}")
  set_property(GLOBAL APPEND PROPERTY BEAM_BUILD_TARGETS ${target})
  set_property(GLOBAL APPEND PROPERTY BEAM_INSTALLED_OUTPUTS
    "${directory}/$<TARGET_FILE_NAME:${target}>")
  get_target_property(type ${target} TYPE)
  if(type STREQUAL "SHARED_LIBRARY")
    set_property(GLOBAL APPEND PROPERTY BEAM_INSTALLED_OUTPUTS
      "${directory}/$<TARGET_LINKER_FILE_NAME:${target}>")
  endif()
endfunction()

function(beam_configure_clean)
  get_property(targets GLOBAL PROPERTY BEAM_BUILD_TARGETS)
  get_property(clean_outputs GLOBAL PROPERTY BEAM_INSTALLED_OUTPUTS)
  foreach(target IN LISTS targets)
    get_target_property(type ${target} TYPE)
    list(APPEND clean_outputs
      "$<TARGET_FILE:${target}>" "$<TARGET_OBJECTS:${target}>")
    if(MSVC)
      get_target_property(target_directory ${target} BINARY_DIR)
      set(compile_directory "${target_directory}/${target}.dir/$<CONFIG>")
      set_target_properties(${target} PROPERTIES COMPILE_PDB_NAME "${target}"
        COMPILE_PDB_OUTPUT_DIRECTORY "${compile_directory}")
      list(APPEND clean_outputs "${compile_directory}/${target}.pdb")
      if(NOT type STREQUAL "STATIC_LIBRARY")
        list(APPEND clean_outputs "$<TARGET_PDB_FILE:${target}>"
          "$<TARGET_FILE_DIR:${target}>/$<TARGET_FILE_BASE_NAME:${target}>.ilk")
      endif()
    endif()
    if(type STREQUAL "SHARED_LIBRARY")
      list(APPEND clean_outputs "$<TARGET_LINKER_FILE:${target}>")
      if(MSVC)
        set(linker_file "$<TARGET_LINKER_FILE:${target}>")
        list(APPEND clean_outputs
          "$<PATH:REPLACE_EXTENSION,LAST_ONLY,${linker_file},.exp>")
      endif()
    endif()
  endforeach()
  configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/clean.cmake"
    CMakeFiles/clean.cmake.in @ONLY)
  file(GENERATE
    OUTPUT "${PROJECT_BINARY_DIR}/CMakeFiles/clean_$<CONFIG>.cmake"
    INPUT "${PROJECT_BINARY_DIR}/CMakeFiles/clean.cmake.in")
endfunction()
