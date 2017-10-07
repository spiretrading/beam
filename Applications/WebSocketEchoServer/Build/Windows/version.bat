pushd %~dp0..\..
mkdir Include
pushd Include
mkdir WebSocketEchoServer
popd
popd
printf "#define WEB_SOCKET_ECHO_SERVER_VERSION """> %~dp0../../Include/WebSocketEchoServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/WebSocketEchoServer/Version.hpp
printf """" >> %~dp0../../Include/WebSocketEchoServer/Version.hpp
printf "\n" >> %~dp0../../Include/WebSocketEchoServer/Version.hpp
