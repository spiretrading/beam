pushd %~dp0..\..
mkdir Include
pushd Include
mkdir ServiceLocator
popd
popd
printf "#define SERVICE_LOCATOR_VERSION """> %~dp0../../Include/ServiceLocator/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ServiceLocator/Version.hpp
printf """" >> %~dp0../../Include/ServiceLocator/Version.hpp
printf "\n" >> %~dp0../../Include/ServiceLocator/Version.hpp
