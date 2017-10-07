pushd %~dp0..\..
mkdir Include
pushd Include
mkdir RegistryServer
popd
popd
printf "#define REGISTRY_SERVER_VERSION """> %~dp0../../Include/RegistryServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/RegistryServer/Version.hpp
printf """" >> %~dp0../../Include/RegistryServer/Version.hpp
printf "\n" >> %~dp0../../Include/RegistryServer/Version.hpp
