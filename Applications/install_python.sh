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
pushd ../Beam/Dependencies/aspen
./install_python.sh "$@"
popd
mkdir -p "$python_directory"
cp "../Beam/Libraries/$config/beam.so" "$python_directory"
