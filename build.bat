@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
SET "DIRECTORY=%~dp0"
SET "EXIT_STATUS=0"
CALL :CreateForwardingScripts || EXIT /B 1
CALL :ParseArgs %* || EXIT /B 1
SET "PARALLEL=1"
IF /I "!CONFIG!" == "clean" SET "PARALLEL=0"
IF /I "!CONFIG!" == "reset" SET "PARALLEL=0"
IF !PARALLEL! EQU 1 (
  CALL :Build Beam %*
  IF !EXIT_STATUS! NEQ 0 EXIT /B !EXIT_STATUS!
)
CALL :Build WebApi %*
IF !EXIT_STATUS! NEQ 0 EXIT /B !EXIT_STATUS!
IF !PARALLEL! EQU 1 (
  SET "BUILD_TEMP=!ROOT!\_build_tmp"
  IF EXIST "!BUILD_TEMP!" RD /S /Q "!BUILD_TEMP!"
  MD "!BUILD_TEMP!"
)
CALL :BuildApp Applications\AdminClient %*
CALL :BuildApp Applications\ClientTemplate %*
CALL :BuildApp Applications\DataStoreProfiler %*
CALL :BuildApp Applications\HttpFileServer %*
CALL :BuildApp Applications\QueryStressTest %*
CALL :BuildApp Applications\QueueStressTest %*
CALL :BuildApp Applications\Scratch %*
CALL :BuildApp Applications\ServiceLocator %*
CALL :BuildApp Applications\ServiceProtocolProfiler %*
CALL :BuildApp Applications\ServletTemplate %*
CALL :BuildApp Applications\UidServer %*
CALL :BuildApp Applications\WebSocketEchoServer %*
IF !PARALLEL! EQU 0 (
  IF !EXIT_STATUS! EQU 0 CALL :Build Beam %*
  EXIT /B !EXIT_STATUS!
)
:WaitLoop
SET "RUNNING=0"
FOR %%F IN ("!BUILD_TEMP!\*.running") DO (
  SET "RUNNING=1"
)
IF !RUNNING! EQU 1 (
  waitfor /T 1 BeamBuildDelay >NUL 2>&1
  GOTO WaitLoop
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
  SET "EXIT_STATUS=1"
)
RD /S /Q "!BUILD_TEMP!"
EXIT /B !EXIT_STATUS!
ENDLOCAL

:ParseArgs
SET "DEPENDENCIES=!ROOT!\Beam\Dependencies"
SET "ARGS="
SET "CONFIG="
SET "IS_DEPENDENCY="
:ParseArgsLoop
SET "ARG=%~1"
IF "!ARG!"=="" (
  IF "!IS_DEPENDENCY!"=="1" (
    ECHO Error: -DD requires a path argument.
    EXIT /B 1
  )
  GOTO ParseArgsDone
)
IF "!IS_DEPENDENCY!"=="1" (
  SET "DEPENDENCIES=!ARG!"
  SET "IS_DEPENDENCY="
) ELSE IF "!ARG!"=="-DD" (
  SET "IS_DEPENDENCY=1"
) ELSE IF "!ARG:~0,4!"=="-DD=" (
  SET "DEPENDENCIES=!ARG:~4!"
  IF "!DEPENDENCIES!"=="" (
    ECHO Error: -DD requires a path argument.
    EXIT /B 1
  )
) ELSE (
  SET ARGS=!ARGS! "%~1"
  SET "CONFIG=!ARG!"
)
SHIFT
GOTO ParseArgsLoop
:ParseArgsDone
FOR %%D IN ("!DEPENDENCIES!") DO (
  SET "DEPENDENCIES=%%~fD"
)
EXIT /B 0

:Build
SET "PROJECT=%~1"
IF NOT EXIST "!PROJECT!" (
  MD "!PROJECT!" || (
    SET "EXIT_STATUS=1"
    EXIT /B 1
  )
)
PUSHD "!PROJECT!" || (
  SET "EXIT_STATUS=1"
  EXIT /B 1
)
CALL "!DIRECTORY!!PROJECT!\build.bat" ^
  -DD="!DEPENDENCIES!" !ARGS!
IF ERRORLEVEL 1 SET "EXIT_STATUS=1"
POPD
EXIT /B 0

:BuildApp
IF !PARALLEL! EQU 0 (
  CALL :Build %*
  EXIT /B 0
)
SET "PROJECT=%~1"
SET "PROJECT_NAME=%~n1"
IF NOT EXIST "!PROJECT!" (
  MD "!PROJECT!" || (
    SET "EXIT_STATUS=1"
    EXIT /B 1
  )
)
>"!BUILD_TEMP!\!PROJECT_NAME!.running" ECHO !PROJECT_NAME!
START /B cmd /c "PUSHD "!ROOT!\!PROJECT!" && CALL "!DIRECTORY!!PROJECT!\build.bat" -DD="!DEPENDENCIES!" !ARGS! && DEL "!BUILD_TEMP!\!PROJECT_NAME!.running" || (ECHO failed > "!BUILD_TEMP!\!PROJECT_NAME!.failed" & DEL "!BUILD_TEMP!\!PROJECT_NAME!.running")" >"!BUILD_TEMP!\!PROJECT_NAME!.log" 2>&1
EXIT /B 0

:CreateForwardingScripts
FOR %%S IN (configure build) DO (
  IF NOT EXIST %%S.bat (
    >%%S.bat ECHO @ECHO OFF
    >>%%S.bat ECHO CALL "%~dp0%%S.bat" %%*
    IF ERRORLEVEL 1 EXIT /B 1
  )
)
IF NOT EXIST Applications MD Applications || EXIT /B 1
IF NOT EXIST Applications\install_python.bat (
  >Applications\install_python.bat ECHO @ECHO OFF
  >>Applications\install_python.bat ECHO CALL ^
    "%~dp0Applications\install_python.bat" %%*
  IF ERRORLEVEL 1 EXIT /B 1
)
FOR %%S IN (setup stress_test) DO (
  IF NOT EXIST Applications\%%S.py (
    >Applications\%%S.py ECHO import runpy
    >>Applications\%%S.py ECHO runpy.run_path(^
      r"%~dp0Applications\%%S.py", run_name='__main__'^)
    IF ERRORLEVEL 1 EXIT /B 1
  )
)
EXIT /B 0
