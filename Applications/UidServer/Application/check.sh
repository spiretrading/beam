#!/bin/bash
APPLICATION="UidServer"
PID_FILE="pid.lock"

is_process_running() {
  local pid=$1
  ps -p "$pid" > /dev/null 2>&1
  return $?
}

if [[ -f "$PID_FILE" ]]; then
  existing_pid=$(<"$PID_FILE")
  if ! is_process_running "$existing_pid"; then
    echo "$APPLICATION is not running."
  fi
else
  echo "$APPLICATION is not running."
fi
