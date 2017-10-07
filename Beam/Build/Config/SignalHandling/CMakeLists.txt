file(GLOB header_files
  ./../../../Include/Beam/SignalHandling/*.hpp)
file(GLOB source_files
  ./../../../Source/SignalHandling/*.cpp)

add_library(SignalHandling ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(SignalHandling PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_signal_handling)
install(TARGETS SignalHandling CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS SignalHandling CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
