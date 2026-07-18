file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/PointersTests/*.cpp)
add_executable(PointersTests ${header_files} ${source_files})
if(MSVC)
  target_precompile_headers(PointersTests PRIVATE <doctest/doctest.h>
    <Beam/Pointers/Pointers.hpp> <Beam/Utilities/Utilities.hpp>)
  set_source_files_properties(${BEAM_SOURCE_PATH}/PointersTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()
add_custom_command(TARGET PointersTests POST_BUILD COMMAND PointersTests)
install(TARGETS PointersTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS PointersTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
