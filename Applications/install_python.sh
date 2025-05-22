#!/bin/bash
if [ "$1" == "" ]; then
  config="Release"
else
  config="$1"
fi
python_directory=$(python3 -m site --user-site)
pushd ../Beam/Dependencies/aspen
./install_python.sh "$@"
popd
mkdir -p "$python_directory"
cp "../Beam/Libraries/$config/beam.so" "$python_directory"
