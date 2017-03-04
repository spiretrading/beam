#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/WebSocketEchoServer
printf "#define WEB_SOCKET_ECHO_SERVER_VERSION \""> ./../../Include/WebSocketEchoServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/WebSocketEchoServer/Version.hpp
printf \" >> ./../../Include/WebSocketEchoServer/Version.hpp
printf "\n" >> ./../../Include/WebSocketEchoServer/Version.hpp
