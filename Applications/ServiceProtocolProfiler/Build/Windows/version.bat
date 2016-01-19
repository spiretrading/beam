cd %~dp0../..
mkdir Include
cd Include
mkdir ServiceProtocolProfiler
cd %~dp0
printf "#define SERVICE_PROTOCOL_PROFILER_VERSION """> %~dp0../../Include/ServiceProtocolProfiler/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/ServiceProtocolProfiler/Version.hpp
printf """" >> %~dp0../../Include/ServiceProtocolProfiler/Version.hpp
