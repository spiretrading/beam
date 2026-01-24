@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
SET "EXIT_STATUS=0"
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF
  >>configure.bat ECHO CALL "%~dp0configure.bat" %%*
)
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF
  >>build.bat ECHO CALL "%~dp0build.bat" %%*
)
CALL :Configure Beam %*
CALL :Configure WebApi %*
CALL :Configure Applications\AdminClient %*
CALL :Configure Applications\ClientTemplate %*
CALL :Configure Applications\DataStoreProfiler %*
CALL :Configure Applications\HttpFileServer %*
CALL :Configure Applications\QueryStressTest %*
CALL :Configure Applications\QueueStressTest %*
CALL :Configure Applications\Scratch %*
CALL :Configure Applications\ServiceLocator %*
CALL :Configure Applications\ServiceProtocolProfiler %*
CALL :Configure Applications\ServletTemplate %*
CALL :Configure Applications\UidServer %*
CALL :Configure Applications\WebSocketEchoServer %*
EXIT /B !EXIT_STATUS!
ENDLOCAL

:Configure
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\configure.bat" -DD="!ROOT!\Beam\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
IF ERRORLEVEL 1 SET "EXIT_STATUS=1"
POPD
EXIT /B 0
