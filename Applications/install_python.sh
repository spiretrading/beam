#!/bin/bash
set -o errexit
set -o pipefail
config="${1:-Release}"
shopt -s nocasematch
case "$config" in
  release) config="Release" ;;
  debug) config="Debug" ;;
  relwithdebinfo) config="RelWithDebInfo" ;;
  minsizerel) config="MinSizeRel" ;;
  *)
    echo "Error: Invalid configuration \"$config\"."
    exit 1
    ;;
esac
shopt -u nocasematch
python_directory=$(python3 -m site --user-site)
aspen_config="Release"
if [[ "$config" == "Debug" ]]; then
  aspen_config="Debug"
fi
pushd ../Beam/Dependencies/aspen
./install_python.sh "$aspen_config"
popd
mkdir -p "$python_directory"
cp "../Beam/Libraries/$config/beam.so" "$python_directory"
