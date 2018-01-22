SETLOCAL
PUSHD ..\..\
CALL npm install
PUSHD node_modules
CALL npm link webpack
POPD
POPD
ENDLOCAL
