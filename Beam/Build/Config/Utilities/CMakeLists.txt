file(GLOB header_files
  ./../../../Include/Beam/Utilities/*.hpp)
file(GLOB source_files
  ./../../../Source/Utilities/*.cpp)

add_library(Utilities ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Utilities PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_utilities)
install(TARGETS Utilities CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS Utilities CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
