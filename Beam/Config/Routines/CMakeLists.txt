file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/RoutinesTests/*.cpp)
add_executable(RoutinesTests ${source_files})
beam_configure_target(RoutinesTests DEPENDENCIES CRYPTOPP)
target_link_libraries(RoutinesTests PUBLIC BeamRoutines)
add_custom_command(TARGET RoutinesTests POST_BUILD COMMAND RoutinesTests)
beam_install_target(RoutinesTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
file(GLOB_RECURSE
  functional_source_files ${BEAM_SOURCE_PATH}/RoutinesFunctionalTests/*.cpp)
add_executable(RoutinesFunctionalTests ${functional_source_files})
beam_configure_target(RoutinesFunctionalTests DEPENDENCIES CRYPTOPP)
target_link_libraries(RoutinesFunctionalTests PUBLIC BeamRoutines)
beam_install_target(
  RoutinesFunctionalTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
