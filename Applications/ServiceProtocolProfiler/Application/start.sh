#!/bin/bash
APPLICATION="ServiceProtocolProfiler"
CONFIG_FILE="config.yml"
LOG_DIR="./logs"
START_LOCK=".start.lock"
if ! mkdir "$START_LOCK" 2> /dev/null; then
  echo "Error: Startup is locked by $START_LOCK." >&2
  exit 1
fi
trap 'rmdir "$START_LOCK"' EXIT
trap 'exit 1' HUP INT TERM
pid=""
status=0
./check.sh > /dev/null || status=$?
if((status == 0)); then
  pid=$(<pid.lock)
elif((status != 1)); then
  exit "$status"
fi
if [[ ! -f "$APPLICATION" || ! -x "$APPLICATION" ]]; then
  echo "Error: $APPLICATION is missing or not executable." >&2
  exit 1
fi
if [[ ! -f "$CONFIG_FILE" ]]; then
  echo "Error: $CONFIG_FILE does not exist." >&2
  exit 1
fi
log_name="srv_*.log"
if [[ -z "$pid" ]]; then
  mkdir -p "$LOG_DIR" || exit 1
  for existing_log in srv_*.log; do
    if [[ -f "$existing_log" ]]; then
      mv "$existing_log" "$LOG_DIR" || exit 1
    fi
  done
  log_name="srv_$(date '+%Y%m%d_%H_%M_%S').log"
  : > "$log_name" || exit 1
  {
    ./"$APPLICATION" > "$log_name" 2>&1 &
    pid=$!
    echo "$pid"
  } > pid.lock || exit 1
fi

deadline=$((SECONDS + 30))
while((SECONDS < deadline)); do
  status=0
  ./check.sh > /dev/null || status=$?
  if((status == 1)); then
    if ! kill -0 "$pid" 2> /dev/null; then
      wait "$pid" 2> /dev/null
      rm -f pid.lock
      echo "Error: $APPLICATION exited during startup; see $log_name." >&2
      exit 1
    fi
    sleep 0.5
    continue
  elif((status != 0)); then
    exit "$status"
  fi
  exit 0
done
echo "Error: $APPLICATION startup timed out; see $log_name." >&2
exit 1
