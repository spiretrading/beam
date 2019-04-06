@ECHO OFF
SETLOCAL
SET ROOT=%cd%
CALL %~dp0Beam\run_cmake.bat
IF NOT EXIST Applications\AdminClient (
  mkdir Applications\AdminClient
)
PUSHD Applications\AdminClient
CALL %~dp0Applications\AdminClient\run_cmake.bat -DD="%ROOT%\Dependencies"
POPD
ENDLOCAL
