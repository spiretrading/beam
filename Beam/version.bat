@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "APP_NAME=%~1"
IF "!APP_NAME!"=="" (
  ECHO Error: Application name required.
  EXIT /B 1
)
SET "VERSION="
FOR /F "usebackq tokens=*" %%a IN (
    `git -C "%~dp0.." rev-list --count --first-parent HEAD`) DO (
  SET "VERSION=%%a"
)
IF NOT DEFINED VERSION EXIT /B 1
FINDSTR /L /X /C:"#define !APP_NAME!_VERSION \"!VERSION!\"" ^
  Version.hpp >NUL 2>&1
IF ERRORLEVEL 1 (
  (ECHO #define !APP_NAME!_VERSION "!VERSION!") >Version.hpp || EXIT /B 1
)
EXIT /B 0
