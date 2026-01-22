@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET ROOT=%cd%
SET EXIT_STATUS=0
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF
  >>configure.bat ECHO CALL "%~dp0configure.bat" %%*
)
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF
  >>build.bat ECHO CALL "%~dp0build.bat" %%*
)
CALL:configure Beam %*
CALL:configure WebApi %*
CALL:configure Applications\AdminClient %*
CALL:configure Applications\ClientTemplate %*
CALL:configure Applications\DataStoreProfiler %*
CALL:configure Applications\HttpFileServer %*
CALL:configure Applications\QueryStressTest %*
CALL:configure Applications\QueueStressTest %*
CALL:configure Applications\Scratch %*
CALL:configure Applications\ServiceLocator %*
CALL:configure Applications\ServiceProtocolProfiler %*
CALL:configure Applications\ServletTemplate %*
CALL:configure Applications\UidServer %*
CALL:configure Applications\WebSocketEchoServer %*
EXIT /B !EXIT_STATUS!
ENDLOCAL

:configure
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\configure.bat" -DD="!ROOT!\Beam\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
IF ERRORLEVEL 1 SET EXIT_STATUS=1
POPD
EXIT /B 0
