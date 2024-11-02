#!/bin/bash
APPLICATION="UidServer"
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
interface=$(yq '.server.interface // ""' "$CONFIG_FILE")
addresses=($(yq '.server.addresses[] // ""' "$CONFIG_FILE"))
all_addresses=()
if [[ -n "$interface" ]]; then
  all_addresses+=("$interface")
fi
if [[ ${#addresses[@]} -gt 0 ]]; then
  all_addresses+=("${addresses[@]}")
fi
if [[ ${#all_addresses[@]} -eq 0 ]]; then
  exit 0
fi

is_any_address_listening() {
  for addr in "${all_addresses[@]}"; do
    host="${addr%%:*}"
    port="${addr##*:}"
    if ss -ltn | grep -qE "$host:$port"; then
      return 0
    fi
  done
  return 1
}

while true; do
  echo "Uid: A"
  if ./check.sh | grep -q "$APPLICATION is not running."; then
    exit 1
  fi
  echo "Uid: B"
  if is_any_address_listening; then
    break
  fi
  echo "Uid: C"
  sleep 0.5
done
