#!/bin/bash
# check_server.sh
# 
# Use this script to display the status of WebSocketEchoServer.
#
# Usage: ./check_server.sh

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
processes=$(ps -ef | grep -i "WebSocketEchoServer" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
check_run=$(awk -v a="$processes" -v b="WebSocketEchoServer" 'BEGIN { print index(a, b) }')
if [ "$check_run" = "0" ]; then
  # WebSocketEchoServer is not running.
  echo "${yellow}WebSocketEchoServer${reset} is ${red}inactive${reset}."
else
  # WebSocketEchoServer is running.
  echo "${yellow}WebSocketEchoServer${reset} is ${green}active${reset}."
fi
echo
