@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET ROOT=%cd%
SET DIRECTORY=%~dp0
SET EXIT_STATUS=0
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF
  >>configure.bat ECHO CALL "%~dp0configure.bat" %%*
)
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF
  >>build.bat ECHO CALL "%~dp0build.bat" %%*
)
SET ARGS=%*
SET FIRST_ARG=%~1
SET PARALLEL=1
IF "!FIRST_ARG!" == "clean" SET PARALLEL=0
IF "!FIRST_ARG!" == "reset" SET PARALLEL=0
CALL:build Beam %*
IF !EXIT_STATUS! NEQ 0 (
  EXIT /B !EXIT_STATUS!
)
CALL:build WebApi %*
IF !EXIT_STATUS! NEQ 0 (
  EXIT /B !EXIT_STATUS!
)
IF !PARALLEL! EQU 1 (
  SET BUILD_TEMP=!ROOT!\_build_tmp
  IF EXIST "!BUILD_TEMP!" RD /S /Q "!BUILD_TEMP!"
  MD "!BUILD_TEMP!"
)
CALL:build_app Applications\AdminClient %*
CALL:build_app Applications\ClientTemplate %*
CALL:build_app Applications\DataStoreProfiler %*
CALL:build_app Applications\HttpFileServer %*
CALL:build_app Applications\QueryStressTest %*
CALL:build_app Applications\QueueStressTest %*
CALL:build_app Applications\Scratch %*
CALL:build_app Applications\ServiceLocator %*
CALL:build_app Applications\ServiceProtocolProfiler %*
CALL:build_app Applications\ServletTemplate %*
CALL:build_app Applications\UidServer %*
CALL:build_app Applications\WebSocketEchoServer %*
IF !PARALLEL! EQU 0 (
  EXIT /B !EXIT_STATUS!
)
:wait_loop
SET RUNNING=0
FOR %%F IN ("!BUILD_TEMP!\*.running") DO SET RUNNING=1
IF !RUNNING! EQU 1 (
  timeout /t 1 /nobreak >NUL
  GOTO wait_loop
)
FOR %%F IN ("!BUILD_TEMP!\*.log") DO (
  IF %%~zF GTR 0 (
    ECHO.
    ECHO ============================================================
    ECHO %%~nF
    ECHO ============================================================
    TYPE "%%F"
  )
)
FOR %%F IN ("!BUILD_TEMP!\*.failed") DO (
  SET EXIT_STATUS=1
)
RD /S /Q "!BUILD_TEMP!"
EXIT /B !EXIT_STATUS!
ENDLOCAL

:build
SET PROJECT=%~1
IF NOT EXIST "!PROJECT!" (
  MD "!PROJECT!"
)
PUSHD "!PROJECT!"
CALL "!DIRECTORY!!PROJECT!\build.bat" -DD="!ROOT!\Beam\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
IF ERRORLEVEL 1 SET EXIT_STATUS=1
POPD
EXIT /B 0

:build_app
IF !PARALLEL! EQU 0 (
  CALL:build %*
  EXIT /B 0
)
SET PROJECT=%~1
SET PROJECT_NAME=%~n1
IF NOT EXIST "!PROJECT!" (
  MD "!PROJECT!"
)
>"!BUILD_TEMP!\!PROJECT_NAME!.running" ECHO !PROJECT_NAME!
START /B cmd /c "PUSHD "!ROOT!\!PROJECT!" && CALL "!DIRECTORY!!PROJECT!\build.bat" -DD="!ROOT!\Beam\Dependencies" !ARGS! && DEL "!BUILD_TEMP!\!PROJECT_NAME!.running" || (DEL "!BUILD_TEMP!\!PROJECT_NAME!.running" & ECHO failed > "!BUILD_TEMP!\!PROJECT_NAME!.failed")" >"!BUILD_TEMP!\!PROJECT_NAME!.log" 2>&1
EXIT /B 0
