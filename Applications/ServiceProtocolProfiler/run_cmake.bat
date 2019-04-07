@ECHO OFF
SETLOCAL
cmake -T host=x64 %* %~dp0
ENDLOCAL
