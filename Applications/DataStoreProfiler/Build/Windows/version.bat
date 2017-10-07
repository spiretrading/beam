pushd %~dp0..\..
mkdir Include
pushd Include
mkdir DataStoreProfiler
popd
popd
printf "#define DATA_STORE_PROFILER_VERSION """> %~dp0../../Include/DataStoreProfiler/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/DataStoreProfiler/Version.hpp
printf """" >> %~dp0../../Include/DataStoreProfiler/Version.hpp
printf "\n" >> %~dp0../../Include/DataStoreProfiler/Version.hpp
