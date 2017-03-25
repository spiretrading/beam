cd %~dp0../..
mkdir Include
cd Include
mkdir UidServer
cd %~dp0
printf "#define UID_SERVER_VERSION """> %~dp0../../Include/UidServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/UidServer/Version.hpp
printf """" >> %~dp0../../Include/UidServer/Version.hpp
printf "\n" >> %~dp0../../Include/UidServer/Version.hpp
