#!/bin/bash
# start_server.sh
# 
# Use this script to start the HttpFileServer.
#
# Usage: ./start_server.sh    Starts the HttpFileServer.

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
directory=$(ls -l)
check_exist=$(awk -v a="$directory" -v b="HttpFileServer" 'BEGIN { print index(a, b) }')
if [ "$check_exist" = "0" ]; then
  # HttpFileServer is not present.
  echo "${red}[ERROR]${reset} Could not start ${yellow}HttpFileServer${reset}."
  echo "        ${yellow}HttpFileServer${reset} could not be found."
else
  # HttpFileServer is present.
  leftover=$(ls srv_*.log 2>/dev/null)
  if [ -n "leftover" ]; then
    if [ ! -d "logs" ]; then
      mkdir logs
    fi
    for var in $leftover; do
      mv $var logs
    done
  fi
  processes=$(ps -ef | grep -i "HttpFileServer" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
  check_run=$(awk -v a="$processes" -v b="HttpFileServer" 'BEGIN { print index(a, b) }')
  if [ "$check_run" = "0" ]; then
    date_time=$(date '+%Y%m%d_%H_%M_%S')
    new_log_name="srv_$date_time.log"
    ./HttpFileServer &> $new_log_name &
    echo "${yellow}HttpFileServer${reset} has been started."
  else
    # HttpFileServer is already running.
    echo "${red}[ERROR]${reset} Could not start ${yellow}HttpFileServer${reset}."
    echo "        ${yellow}HttpFileServer${reset} is already running."
  fi
fi
echo
