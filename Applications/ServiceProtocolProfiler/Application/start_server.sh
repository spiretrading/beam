#!/bin/bash
# start_server.sh
# 
# Use this script to start the ServletTemplate.
#
# Usage: ./start_server.sh    Starts the ServletTemplate.

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
directory=$(ls -l)
check_exist=$(awk -v a="$directory" -v b="ServletTemplate" 'BEGIN { print index(a, b) }')
if [ "$check_exist" = "0" ]; then
  # ServletTemplate is not present.
  echo "${red}[ERROR]${reset} Could not start ${yellow}ServletTemplate${reset}."
  echo "        ${yellow}ServletTemplate${reset} could not be found."
else
  # ServletTemplate is present.
  leftover=$(ls srv_*.log 2>/dev/null)
  if [ -n "leftover" ]; then
    if [ ! -d "logs" ]; then
      mkdir logs
    fi
    for var in $leftover; do
      mv $var logs
    done
  fi
  processes=$(ps -ef | grep -i "ServletTemplate" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
  check_run=$(awk -v a="$processes" -v b="ServletTemplate" 'BEGIN { print index(a, b) }')
  if [ "$check_run" = "0" ]; then
    date_time=$(date '+%Y%m%d_%H_%M_%S')
    new_log_name="srv_$date_time.log"
    ./ServletTemplate > $new_log_name &
    echo "${yellow}ServletTemplate${reset} has been started."
  else
    # ServletTemplate is already running.
    echo "${red}[ERROR]${reset} Could not start ${yellow}ServletTemplate${reset}."
    echo "        ${yellow}ServletTemplate${reset} is already running."
  fi
fi
echo
