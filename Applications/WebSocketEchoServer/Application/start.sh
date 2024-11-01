#!/bin/bash
APPLICATION="WebSocketEchoServer"
CONFIG_FILE="config.yml"
LOG_DIR="./logs"
mkdir -p "$LOG_DIR"
date_time=$(date '+%Y%m%d_%H_%M_%S')
log_name="srv_$date_time.log"
for existing_log in srv_*.log; do
  if [[ -f "$existing_log" ]]; then
    mv "$existing_log" "$LOG_DIR"
  fi
done
if ! (./check.sh | grep -q "$APPLICATION is not running."); then
  exit 0
fi
./$APPLICATION > "$log_name" 2>&1 &
new_pid=$!
echo "$new_pid" > "pid.lock"
interface=$(yq '.interface // empty' "$CONFIG_FILE")
all_addresses=()
if [[ -n "$interface" ]]; then
  all_addresses+=("$interface")
fi
if [[ ${#all_addresses[@]} -eq 0 ]]; then
  exit 0
fi

is_any_address_listening() {
  for addr in "${all_addresses[@]}"; do
    host="${addr%%:*}"
    port="${addr##*:}"
    if ss -ltn | grep -qE "$host:$port"; then
      return 1
    fi
  done
  return 0
}

while true; do
  if ./check.sh | grep -q "$APPLICATION is not running."; then
    exit 1
  fi
  if is_any_address_listening; then
    break
  fi
  sleep 0.5
done
