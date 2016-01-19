#!/bin/bash
# check_server.sh
# 
# Use this script to display the status of UidServer.
#
# Usage: ./check_server.sh

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
processes=$(ps -ef | grep -i "UidServer" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
check_run=$(awk -v a="$processes" -v b="UidServer" 'BEGIN { print index(a, b) }')
if [ "$check_run" = "0" ]; then
  # UidServer is not running.
  echo "${yellow}UidServer${reset} is ${red}inactive${reset}."
else
  # UidServer is running.
  echo "${yellow}UidServer${reset} is ${green}active${reset}."
fi
echo
