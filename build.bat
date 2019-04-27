@ECHO OFF
SETLOCAL
SET ROOT=%cd%
CALL:build Beam %*
CALL:build WebApi %*
CALL:build Applications\AdminClient %*
CALL:build Applications\ClientTemplate %*
CALL:build Applications\DataStoreProfiler %*
CALL:build Applications\HttpFileServer %*
CALL:build Applications\QueryStressTest %*
CALL:build Applications\RegistryServer %*
CALL:build Applications\ServiceLocator %*
CALL:build Applications\ServiceProtocolProfiler %*
CALL:build Applications\ServletTemplate %*
CALL:build Applications\UidServer %*
CALL:build Applications\WebSocketEchoServer %*
ENDLOCAL
EXIT /B %ERRORLEVEL%

:build
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\build.bat" %~2 %~3 %~4 %~5 %~6 %~7
POPD
EXIT /B 0
