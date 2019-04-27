@ECHO OFF
SETLOCAL
SET ROOT=%cd%
IF NOT EXIST build.bat (
  ECHO @ECHO OFF > build.bat
  ECHO CALL "%~dp0build.bat" %%* >> build.bat
)
IF NOT EXIST configure.bat (
  ECHO @ECHO OFF > configure.bat
  ECHO CALL "%~dp0configure.bat" %%* >> configure.bat
)
IF NOT "%~dp0" == "%ROOT%\" (
  IF EXIST source (
    RMDIR source /S /Q
  )
  mklink /j source "%~dp0source" >NUL
)
ENDLOCAL
