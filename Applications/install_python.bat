@ECHO OFF
IF "%~1" == "" (
  SET CONFIG=Release
) ELSE (
  SET CONFIG=%~1
)
SET PYTHON_PATH=
FOR /f "delims=" %%i IN ('python -m site --user-site 2^>nul') DO SET PYTHON_PATH=%%i
IF "%PYTHON_PATH%" == "" (
  ECHO Error: Unable to retrieve Python user-site path.
  EXIT /B 1
)
PUSHD ..\Beam\Dependencies\aspen
CALL install_python.bat %*
POPD
IF NOT EXIST "%PYTHON_PATH%" (
  MKDIR "%PYTHON_PATH%"
  IF %ERRORLEVEL% GEQ 1 (
    ECHO Error: Unable to create directory "%PYTHON_PATH%".
    EXIT /B 1
  )
)
IF NOT EXIST "%PYTHON_PATH%\beam" (
  MKDIR "%PYTHON_PATH%\beam"
  IF %ERRORLEVEL% GEQ 1 (
    ECHO Error: Unable to create directory "%PYTHON_PATH%\beam".
    EXIT /B 1
  )
)
IF NOT EXIST "Python\__init__.py" (
  ECHO Error: Source file "Python\__init__.py" not found.
  EXIT /B 1
)
COPY "Python\__init__.py" "%PYTHON_PATH%\beam" >nul
IF %ERRORLEVEL% GEQ 1 (
  ECHO Error: Failed to copy "__init__.py" to "%PYTHON_PATH%\beam".
  EXIT /B 1
)
IF NOT EXIST "..\Beam\Libraries\%CONFIG%\_beam.pyd" (
  ECHO Error: Source file "..\Beam\Libraries\%CONFIG%\_beam.pyd" not found.
  EXIT /B 1
)
COPY "..\Beam\Libraries\%CONFIG%\_beam.pyd" "%PYTHON_PATH%\beam" >nul
IF %ERRORLEVEL% GEQ 1 (
  ECHO Error: Failed to copy "_beam.pyd" to "%PYTHON_PATH%\beam".
  EXIT /B 1
)
EXIT /B 0
