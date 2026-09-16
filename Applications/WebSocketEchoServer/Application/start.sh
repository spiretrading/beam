#!/bin/bash
APPLICATION="WebSocketEchoServer"
CONFIG_FILE="config.yml"
LOG_DIR="./logs"
status=0
./check.sh > /dev/null || status=$?
if((status == 0)); then
  exit 0
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
platform=$(uname -s) || exit 1
if [[ "$platform" == "Linux" ]]; then
  listener_command=ss
else
  listener_command=lsof
fi
for dependency in yq "$listener_command"; do
  if ! command -v "$dependency" > /dev/null; then
    echo "Error: $dependency is required to start $APPLICATION." >&2
    exit 1
  fi
done
interface=$(yq -r '.interface // empty' "$CONFIG_FILE") || exit 1
port=${interface##*:}
if [[ "$interface" != *:* || ! "$port" =~ ^[0-9]{1,5}$ ]]; then
  echo "Error: $CONFIG_FILE must specify an interface with a TCP port." >&2
  exit 1
fi
port=$((10#$port))
if((port < 1 || port > 65535)); then
  echo "Error: Invalid TCP port in $CONFIG_FILE." >&2
  exit 1
fi
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

is_listening() {
  local sockets
  if [[ "$platform" == "Linux" ]]; then
    sockets=$(ss -ltnpH) || return 2
    awk -v pid="$pid" -v port="$port" '
      $4 ~ (":" port "$") && index($0, "pid=" pid ",") { found = 1 }
      END { exit !found }' <<< "$sockets"
  else
    lsof -a -p "$pid" -iTCP:"$port" -sTCP:LISTEN -t > /dev/null 2>&1
  fi
}

deadline=$((SECONDS + 30))
while((SECONDS < deadline)); do
  status=0
  ./check.sh > /dev/null || status=$?
  if((status == 1)); then
    if ! kill -0 "$pid" 2> /dev/null; then
      wait "$pid"
      rm -f pid.lock
      echo "Error: $APPLICATION exited during startup; see $log_name." >&2
      exit 1
    fi
    sleep 0.5
    continue
  elif((status != 0)); then
    exit "$status"
  fi
  status=0
  is_listening || status=$?
  if((status == 0)); then
    exit 0
  elif((status != 1)); then
    echo "Error: Unable to check $APPLICATION's listener; see $log_name." >&2
    exit "$status"
  fi
  sleep 0.5
done
echo "Error: $APPLICATION startup timed out; see $log_name." >&2
exit 1
