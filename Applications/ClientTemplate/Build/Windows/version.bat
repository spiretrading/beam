pushd %~dp0..\..
mkdir Include
pushd Include
mkdir ClientTemplate
popd
popd
printf "#define CLIENT_TEMPLATE_VERSION """> %~dp0../../Include/ClientTemplate/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ClientTemplate/Version.hpp
printf """" >> %~dp0../../Include/ClientTemplate/Version.hpp
printf "\n" >> %~dp0../../Include/ClientTemplate/Version.hpp
