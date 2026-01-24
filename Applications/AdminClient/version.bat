@ECHO OFF
SETLOCAL EnableDelayedExpansion
IF NOT EXIST Version.hpp (
  COPY NUL Version.hpp >NUL
)
FOR /F "usebackq tokens=*" %%a IN (`git --git-dir=%~dp0..\..\.git rev-list --count --first-parent HEAD`) DO SET VERSION=%%a
FINDSTR "!VERSION!" Version.hpp >NUL
IF ERRORLEVEL 1 (
  >Version.hpp ECHO #define ADMIN_CLIENT_VERSION "!VERSION!"
)
EXIT /B 0
