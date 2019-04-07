@ECHO OFF
SETLOCAL
SET ROOT=%cd%
CALL %~dp0Beam\run_cmake.bat %*
CALL:run_cmake AdminClient %*
CALL:run_cmake ClientTemplate %*
CALL:run_cmake DataStoreProfiler %*
CALL:run_cmake HttpFileServer %*
CALL:run_cmake QueryStressTest %*
CALL:run_cmake RegistryServer %*
CALL:run_cmake ServiceLocator %*
CALL:run_cmake ServiceProtocolProfiler %*
CALL:run_cmake ServletTemplate %*
CALL:run_cmake UidServer %*
CALL:run_cmake WebSocketEchoServer %*
ENDLOCAL
EXIT /B %ERRORLEVEL%

:run_cmake
IF NOT EXIST Applications\%~1 (
  mkdir Applications\%~1
)
PUSHD Applications\%~1
CALL %~dp0Applications\%~1\run_cmake.bat -DD="%ROOT%\Dependencies"
POPD
EXIT /B 0
