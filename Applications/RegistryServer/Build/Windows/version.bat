cd %~dp0../..
mkdir Include
cd Include
mkdir RegistryServer
cd %~dp0
printf "#define REGISTRY_SERVER_VERSION """> %~dp0../../Include/RegistryServer/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/RegistryServer/Version.hpp
printf """" >> %~dp0../../Include/RegistryServer/Version.hpp
