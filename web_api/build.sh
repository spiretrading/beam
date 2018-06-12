#!/bin/bash
if [ $# -eq 0 ] || [ "$1" != "Debug" ]; then
  export PROD_ENV=1
fi
if [ ! -d "node_modules" ]; then
  UPDATE_NODE=1
else
  pushd node_modules
  if [ ! -f "mod_time.txt" ]; then
    UPDATE_NODE=1
  else
    pt="$(ls -l --time-style=full-iso ../package.json | awk '{print $6 $7}')"
    mt="$(ls -l --time-style=full-iso mod_time.txt | awk '{print $6 $7}')"
    if [ "$pt" \> "$mt" ]; then
      UPDATE_NODE=1
    fi
  fi
  popd
fi
if [ "$UPDATE_NODE" = "1" ]; then
  UPDATE_LIBRARY=1
  npm install
  pushd node_modules
  echo "timestamp" > mod_time.txt
  popd
fi
if [ ! -d "library" ]; then
  UPDATE_LIBRARY=1
else
  st="$(find source/ -type f | xargs ls -l --time-style=full-iso | awk '{print $6 $7}' | sort -r | head -1)"
  lt="$(find library/ -type f | xargs ls -l --time-style=full-iso | awk '{print $6 $7}' | sort -r | head -1)"
  if [ "$st" \> "$lt" ]; then
    UPDATE_LIBRARY=1
  fi
fi
if [ "$UPDATE_LIBRARY" = "1" ]; then
  if [ -d library ]; then
    rm -rf library
  fi
  node ./node_modules/webpack/bin/webpack.js
fi
