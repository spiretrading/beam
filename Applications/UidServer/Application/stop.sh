#!/bin/bash
APPLICATION="UidServer"
CHECK_SCRIPT="./check.sh"
PID_FILE="pid.lock"
TIMEOUT="300s"

is_application_running() {
  $CHECK_SCRIPT > /dev/null
  return $?
}

if [[ -f "$PID_FILE" ]]; then
  pid=$(<"$PID_FILE")
  if is_application_running > /dev/null; then
    kill -SIGINT "$pid" 2> /dev/null
    timeout "$TIMEOUT" wait "$pid" 2> /dev/null || true
    if is_application_running > /dev/null; then
      kill -SIGKILL "$pid"
      log_file=$(ls -t srv_*.log 2>/dev/null | head -n 1)
      if [[ -n "$log_file" ]]; then
       echo "Forcefully terminated $APPLICATION." >> "$log_file"
      fi
    fi
  fi
  rm -f "$PID_FILE"
fi
