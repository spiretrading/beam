#!/bin/bash
# stop_server.sh
# 
# Use this script to stop a RegistryServer.
#
# Usage: ./stop_server.sh    Stops the RegistryServer.

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
# RegistryServer is present.
processes=$(ps -ef | grep -i "RegistryServer" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
check_run=$(awk -v a="$processes" -v b="RegistryServer" 'BEGIN { print index(a, b) }')
if [ "$check_run" = "0" ]; then
  # RegistryServer is not running.
  echo "${red}[ERROR]${reset} Could not terminate ${yellow}RegistryServer${reset}."
  echo "        ${yellow}RegistryServer${reset} is not active."
else
  # RegistryServer is already running.
  pid=$(ps -ef | grep -i "RegistryServer" | grep -v "grep" | grep -v "bash" | awk '{ print $2 }')
  kill -9 $pid
  echo "${yellow}RegistryServer${reset} terminated."
  srv_logs=$(ls srv_*.log 2>/dev/null)
  if [ -n "srv_logs" ]; then
    if [ ! -d "logs" ]; then
      mkdir logs
    fi
    for var in $srv_logs; do
      mv $var logs
    done
  else
    echo "${red}[ERROR]${reset} No log file could be found."
  fi
fi
echo
