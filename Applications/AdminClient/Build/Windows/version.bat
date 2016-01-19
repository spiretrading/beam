cd %~dp0../..
mkdir Include
cd Include
mkdir AdminClient
cd %~dp0
printf "#define ADMIN_CLIENT_VERSION """> %~dp0../../Include/AdminClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/AdminClient/Version.hpp
printf """" >> %~dp0../../Include/AdminClient/Version.hpp
printf "\n" >> %~dp0../../Include/AdminClient/Version.hpp
