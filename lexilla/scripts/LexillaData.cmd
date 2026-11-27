@echo off
setlocal DisableDelayedExpansion
pushd "%~dp0" || exit /b 1

set "_VENV_=%~dp0..\..\.venv\Scripts"
set "_ACTIVATED_="
if not exist "%~dp0..\..\.venv\." goto :system_python
call "%_VENV_%\activate.bat" || goto :activation_failed
set "_ACTIVATED_=1"
set "_PYTHON_=%_VENV_%\python.exe"
goto :run

:system_python
set "_PYTHON_=python.exe"
where python.exe >nul 2>&1
if not errorlevel 1 goto :run
if not defined PYTHONHOME goto :no_python
set "_PYTHON_=%PYTHONHOME%\python.exe"
if exist "%_PYTHON_%" goto :run

:no_python
echo Python not found: no .venv or python.exe on PATH or in PYTHONHOME. >&2
popd
exit /b 1

:activation_failed
echo Failed to activate .venv. >&2
popd
exit /b 1

:run
"%_PYTHON_%" "%~dpn0.py"
set "_EXITCODE_=%ERRORLEVEL%"
if defined _ACTIVATED_ call "%_VENV_%\deactivate.bat" || if "%_EXITCODE_%"=="0" set "_EXITCODE_=1"
popd
endlocal & exit /b %_EXITCODE_%
