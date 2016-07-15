cd %~dp0../..
mkdir Include
cd Include
mkdir HttpFileServer
cd %~dp0
printf "#define HTTP_FILE_SERVER_VERSION """> %~dp0../../Include/HttpFileServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/HttpFileServer/Version.hpp
printf """" >> %~dp0../../Include/HttpFileServer/Version.hpp
printf "\n" >> %~dp0../../Include/HttpFileServer/Version.hpp
