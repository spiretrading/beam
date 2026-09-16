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

function(beam_install_target target directory)
  install(TARGETS ${target} DESTINATION "${directory}/$<CONFIG>")
  set_property(GLOBAL APPEND PROPERTY BEAM_BUILD_TARGETS ${target})
  set_property(GLOBAL APPEND PROPERTY BEAM_INSTALLED_OUTPUTS
    "${directory}/$<CONFIG>/$<TARGET_FILE_NAME:${target}>")
  get_target_property(type ${target} TYPE)
  if(type STREQUAL "SHARED_LIBRARY")
    set_property(GLOBAL APPEND PROPERTY BEAM_INSTALLED_OUTPUTS
      "${directory}/$<CONFIG>/$<TARGET_LINKER_FILE_NAME:${target}>")
  endif()
endfunction()

function(beam_configure_clean)
  get_property(targets GLOBAL PROPERTY BEAM_BUILD_TARGETS)
  list(APPEND targets Beam)
  get_property(clean_outputs GLOBAL PROPERTY BEAM_INSTALLED_OUTPUTS)
  set(clean_tracking_directories)
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
    if(CMAKE_GENERATOR MATCHES "^Visual Studio ")
      set(directory "$<TARGET_PROPERTY:${target},BINARY_DIR>")
      list(APPEND clean_tracking_directories
        "${directory}/${target}.dir/$<CONFIG>")
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
  configure_file(Config/clean.cmake CMakeFiles/clean.cmake.in @ONLY)
  file(GENERATE
    OUTPUT "${PROJECT_BINARY_DIR}/CMakeFiles/beam_clean_$<CONFIG>.cmake"
    INPUT "${PROJECT_BINARY_DIR}/CMakeFiles/clean.cmake.in")
endfunction()
