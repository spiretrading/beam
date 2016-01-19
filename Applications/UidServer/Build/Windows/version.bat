cd %~dp0../..
mkdir Include
cd Include
mkdir UidServer
cd %~dp0
printf "#define UID_SERVER_VERSION """> %~dp0../../Include/UidServer/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/UidServer/Version.hpp
printf """" >> %~dp0../../Include/UidServer/Version.hpp
