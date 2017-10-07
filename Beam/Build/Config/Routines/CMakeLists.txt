file(GLOB header_files
  ./../../../Include/Beam/Routines/*.hpp
  ./../../../Include/Beam/Routines/*.inl)
file(GLOB source_files
  ./../../../Source/Routines/*.cpp)

add_library(Routines ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Routines PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_routines)
install(TARGETS Routines CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS Routines CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
