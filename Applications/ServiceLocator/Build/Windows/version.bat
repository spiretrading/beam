cd %~dp0../..
mkdir Include
cd Include
mkdir ServiceLocator
cd %~dp0
printf "#define SERVICE_LOCATOR_VERSION """> %~dp0../../Include/ServiceLocator/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/ServiceLocator/Version.hpp
printf """" >> %~dp0../../Include/ServiceLocator/Version.hpp
