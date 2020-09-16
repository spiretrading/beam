#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
root=$(pwd -P)
if [ ! -f "build.sh" ]; then
  ln -s "$directory/build.sh" build.sh
fi
if [ ! -f "configure.sh" ]; then
  ln -s "$directory/configure.sh" configure.sh
fi
if [ "$directory" != "$root" ]; then
  if [ ! -d "source" ]; then
    ln -s "$directory/source" source
  fi
  if [ ! -f "package.json" ]; then
    ln -s "$directory/package.json" package.json
  fi
  if [ ! -f "tsconfig.json" ]; then
    ln -s "$directory/tsconfig.json" tsconfig.json
  fi
fi
