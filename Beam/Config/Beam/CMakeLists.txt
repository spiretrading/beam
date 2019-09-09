file(GLOB codecs_header_files ${BEAM_INCLUDE_PATH}/Beam/Codecs/*.hpp)
source_group("Codecs" FILES ${codecs_header_files})
file(GLOB collections_header_files ${BEAM_INCLUDE_PATH}/Beam/Collections/*.hpp)
source_group("Collections" FILES ${collections_header_files})
file(GLOB io_header_files ${BEAM_INCLUDE_PATH}/Beam/IO/*.hpp)
source_group("IO" FILES ${io_header_files})
file(GLOB json_header_files ${BEAM_INCLUDE_PATH}/Beam/Json/*.hpp)
source_group("Json" FILES ${json_header_files})
file(GLOB sql_header_files ${BEAM_INCLUDE_PATH}/Beam/Sql/*.hpp)
source_group("Sql" FILES ${sql_header_files})
file(GLOB network_header_files ${BEAM_INCLUDE_PATH}/Beam/Network/*.hpp)
source_group("Network" FILES ${network_header_files})
file(GLOB parsers_header_files ${BEAM_INCLUDE_PATH}/Beam/Parsers/*.hpp)
source_group("Parsers" FILES ${parsers_header_files})
file(GLOB pointers_header_files ${BEAM_INCLUDE_PATH}/Beam/Pointers/*.hpp)
source_group("Pointers" FILES ${pointers_header_files})
file(GLOB queries_header_files ${BEAM_INCLUDE_PATH}/Beam/Queries/*.hpp)
source_group("Queries" FILES ${queries_header_files})
file(GLOB queues_header_files ${BEAM_INCLUDE_PATH}/Beam/Queues/*.hpp)
source_group("Queues" FILES ${queues_header_files})
file(GLOB reactors_header_files ${BEAM_INCLUDE_PATH}/Beam/Reactors/*.hpp)
source_group("Reactors" FILES ${reactors_header_files})
file(GLOB registry_service_header_files
  ${BEAM_INCLUDE_PATH}/Beam/RegistryService/*.hpp)
source_group("RegistryService" FILES ${registry_service_header_files})
file(GLOB routines_header_files ${BEAM_INCLUDE_PATH}/Beam/Routines/*.hpp
  ${BEAM_INCLUDE_PATH}/Beam/Routines/*.inl)
source_group("Routines" FILES ${routines_header_files})
file(GLOB serialization_header_files
  ${BEAM_INCLUDE_PATH}/Beam/Serialization/*.hpp)
source_group("Serialization" FILES ${serialization_header_files})
file(GLOB service_locator_header_files
  ${BEAM_INCLUDE_PATH}/Beam/ServiceLocator/*.hpp)
source_group("ServiceLocator" FILES ${service_locator_header_files})
file(GLOB services_header_files ${BEAM_INCLUDE_PATH}/Beam/Services/*.hpp)
source_group("Services" FILES ${services_header_files})
file(GLOB signal_handling_header_files
  ${BEAM_INCLUDE_PATH}/Beam/SignalHandling/*.hpp)
source_group("SignalHandling" FILES ${signal_handling_header_files})
file(GLOB stomp_header_files ${BEAM_INCLUDE_PATH}/Beam/Stomp/*.hpp)
source_group("Stomp" FILES ${stomp_header_files})
file(GLOB threading_header_files ${BEAM_INCLUDE_PATH}/Beam/Threading/*.hpp)
source_group("Threading" FILES ${threading_header_files})
file(GLOB time_service_header_files ${BEAM_INCLUDE_PATH}/Beam/TimeService/*.hpp)
source_group("TimeService" FILES ${time_service_header_files})
file(GLOB uid_service_header_files ${BEAM_INCLUDE_PATH}/Beam/UidService/*.hpp)
source_group("UidService" FILES ${uid_service_header_files})
file(GLOB utilities_header_files ${BEAM_INCLUDE_PATH}/Beam/Utilities/*.hpp)
source_group("Utilities" FILES ${utilities_header_files})
file(GLOB web_services_header_files ${BEAM_INCLUDE_PATH}/Beam/WebServices/*.hpp)
source_group("WebServices" FILES ${web_services_header_files})
file(GLOB header_files ${codecs_header_files} ${collections_header_files}
  ${io_header_files} ${json_header_files} ${sql_header_files}
  ${network_header_files} ${parsers_header_files} ${pointers_header_files}
  ${queries_header_files} ${queues_header_files} ${reactors_header_files}
  ${registry_service_header_files} ${routines_header_files}
  ${serialization_header_files} ${service_locator_header_files}
  ${services_header_files} ${signal_handling_header_files} ${stomp_header_files}
  ${threading_header_files} ${time_service_header_files}
  ${uid_service_header_files} ${utilities_header_files}
  ${web_services_header_files})
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
add_library(Beam ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Beam PROPERTIES STATIC_LIBRARY_FLAGS_RELEASE
  "${CMAKE_LIBRARY_FLAGS}" LINKER_LANGUAGE CXX OUTPUT_NAME Beam)
