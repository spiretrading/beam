pushd %~dp0..\..
mkdir Include
pushd Include
mkdir AdminClient
popd
popd
printf "#define ADMIN_CLIENT_VERSION """> %~dp0../../Include/AdminClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/AdminClient/Version.hpp
printf """" >> %~dp0../../Include/AdminClient/Version.hpp
printf "\n" >> %~dp0../../Include/AdminClient/Version.hpp
