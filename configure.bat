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
CALL:configure Beam %*
CALL:configure WebApi %*
CALL:configure Applications\AdminClient %*
CALL:configure Applications\ClientTemplate %*
CALL:configure Applications\DataStoreProfiler %*
CALL:configure Applications\HttpFileServer %*
CALL:configure Applications\QueryStressTest %*
CALL:configure Applications\RegistryServer %*
CALL:configure Applications\Scratch %*
CALL:configure Applications\ServiceLocator %*
CALL:configure Applications\ServiceProtocolProfiler %*
CALL:configure Applications\ServletTemplate %*
CALL:configure Applications\UidServer %*
CALL:configure Applications\WebSocketEchoServer %*
ENDLOCAL
EXIT /B %ERRORLEVEL%

:configure
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\configure.bat" -DD="%ROOT%\Beam\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
POPD
EXIT /B 0
