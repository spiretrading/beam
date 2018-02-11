SETLOCAL
IF [%1] == [] (
  SET config=Debug
) ELSE (
  SET config="%1"
)
PUSHD ..\..\
rm -rf library
node .\node_modules\webpack\bin\webpack.js
POPD
ENDLOCAL
