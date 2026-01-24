@ECHO OFF
CALL "%~dp0..\..\Beam\build.bat" -D "%~dp0" %*
EXIT /B %ERRORLEVEL%
