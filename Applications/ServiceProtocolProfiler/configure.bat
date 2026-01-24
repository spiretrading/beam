@ECHO OFF
CALL "%~dp0..\..\Beam\configure.bat" -D "%~dp0" %*
EXIT /B %ERRORLEVEL%
