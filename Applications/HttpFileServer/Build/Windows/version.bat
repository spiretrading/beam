pushd %~dp0..\..
mkdir Include
pushd Include
mkdir HttpFileServer
popd
popd
printf "#define HTTP_FILE_SERVER_VERSION """> %~dp0../../Include/HttpFileServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/HttpFileServer/Version.hpp
printf """" >> %~dp0../../Include/HttpFileServer/Version.hpp
printf "\n" >> %~dp0../../Include/HttpFileServer/Version.hpp
