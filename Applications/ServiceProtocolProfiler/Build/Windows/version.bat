pushd %~dp0..\..
mkdir Include
pushd Include
mkdir ServiceProtocolProfiler
popd
popd
printf "#define SERVICE_PROTOCOL_PROFILER_VERSION """> %~dp0../../Include/ServiceProtocolProfiler/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ServiceProtocolProfiler/Version.hpp
printf """" >> %~dp0../../Include/ServiceProtocolProfiler/Version.hpp
printf "\n" >> %~dp0../../Include/ServiceProtocolProfiler/Version.hpp
