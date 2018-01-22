SETLOCAL
IF [%1] == [] (
  SET config=Debug
) ELSE (
  SET config="%1"
)
PUSHD ..\..\
webpack
POPD
ENDLOCAL
