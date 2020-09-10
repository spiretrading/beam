@ECHO OFF
SETLOCAL
SET ROOT=%cd%
IF NOT EXIST configure.bat (
  ECHO @ECHO OFF > configure.bat
  ECHO CALL "%~dp0configure.bat" %%* >> configure.bat
)
IF NOT EXIST build.bat (
  ECHO @ECHO OFF > build.bat
  ECHO CALL "%~dp0build.bat" %%* >> build.bat
)
CALL:build Beam %*
CALL:build WebApi %*
CALL:build Applications\AdminClient %*
CALL:build Applications\ClientTemplate %*
CALL:build Applications\DataStoreProfiler %*
CALL:build Applications\HttpFileServer %*
CALL:build Applications\QueryStressTest %*
CALL:build Applications\QueueStressTest %*
CALL:build Applications\RegistryServer %*
CALL:build Applications\Scratch %*
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
CALL "%~dp0%~1\build.bat" -DD="%ROOT%\Beam\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
POPD
EXIT /B 0
