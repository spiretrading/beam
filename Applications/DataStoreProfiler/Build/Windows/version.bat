cd %~dp0../..
mkdir Include
cd Include
mkdir DataStoreProfiler
cd %~dp0
printf "#define DATA_STORE_PROFILER_VERSION """> %~dp0../../Include/DataStoreProfiler/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/DataStoreProfiler/Version.hpp
printf """" >> %~dp0../../Include/DataStoreProfiler/Version.hpp
printf "\n" >> %~dp0../../Include/DataStoreProfiler/Version.hpp
