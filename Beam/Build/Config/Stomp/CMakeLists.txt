file(GLOB header_files
  ./../../../Include/Beam/Stomp/*.hpp)
file(GLOB source_files
  ./../../../Source/Stomp/*.cpp)

add_library(Stomp ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Stomp PROPERTIES OUTPUT_NAME beam_stomp)
install(TARGETS Stomp CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS Stomp CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
