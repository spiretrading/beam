#!/bin/bash
DIRECTORY="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd -P)"
"$DIRECTORY/../../Beam/configure.sh" -D="$DIRECTORY" "$@"
