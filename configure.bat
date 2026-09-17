@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
SET "EXIT_STATUS=0"
CALL :ParseArgs %* || EXIT /B 1
CALL :CreateForwardingScripts || EXIT /B 1
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

:ParseArgs
SET "DEPENDENCIES=!ROOT!\Beam\Dependencies"
SET "ARGS="
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
)
SHIFT
GOTO ParseArgsLoop
:ParseArgsDone
FOR %%D IN ("!DEPENDENCIES!") DO (
  SET "DEPENDENCIES=%%~fD"
)
EXIT /B 0

:Configure
IF NOT EXIST "%~1" (
  MD "%~1" || (
    SET "EXIT_STATUS=1"
    EXIT /B 1
  )
)
PUSHD "%~1" || (
  SET "EXIT_STATUS=1"
  EXIT /B 1
)
CALL "%~dp0%~1\configure.bat" -DD="!DEPENDENCIES!" !ARGS!
IF ERRORLEVEL 1 SET "EXIT_STATUS=1"
POPD
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
