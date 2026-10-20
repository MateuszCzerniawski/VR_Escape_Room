@echo off
setlocal enabledelayedexpansion

:: === KONFIGURACJA ===
set "INSTALL_DIR=%~dp0TheForgottenPyramid\Plugins\RemotePython\FlaskServer"
set "EMBED_PYTHON_URL=https://www.python.org/ftp/python/3.12.0/python-3.12.0-embed-amd64.zip"
set "EMBED_ZIP=%INSTALL_DIR%\python-embed.zip"
set "EMBED_DIR=%INSTALL_DIR%\Embedded"

:: === UTWÓRZ FOLDER I PRZEJDŹ DO NIEGO ===
mkdir "%INSTALL_DIR%" 2>nul
cd /d "%INSTALL_DIR%"

:: === 1. POBIERZ EMBED PYTHONA ===
echo [1/4] Pobieranie embedded Pythona...
curl -L -o "%EMBED_ZIP%" "%EMBED_PYTHON_URL%"

echo Rozpakowywanie do %EMBED_DIR%...
powershell -Command "Expand-Archive -Path '%EMBED_ZIP%' -DestinationPath '%EMBED_DIR%' -Force"

:: === 2. EDYTUJ python312._pth ===
echo [2/4] Modyfikacja python312._pth...
set "PTH_FILE=%EMBED_DIR%\python312._pth"
if exist "%PTH_FILE%" (
    >> "%PTH_FILE%" echo ..
    >> "%PTH_FILE%" echo Lib
    >> "%PTH_FILE%" echo Lib/site-packages
    powershell -Command "(Get-Content -Path '%PTH_FILE%') -replace '^#?import site', 'import site' | Set-Content -Path '%PTH_FILE%'"
) else (
    echo Nie znaleziono pliku python312._pth — przerwano.
    exit /b 1
)

:: === 3. INSTALACJA PIP ===
echo [3/4] Pobieranie i instalacja pip...
curl -O https://bootstrap.pypa.io/get-pip.py
"%EMBED_DIR%\python.exe" get-pip.py

:: === 4. INSTALACJA BIBLIOTEK ===
echo [4/4] Instalacja bibliotek z requirements.txt...
"%EMBED_DIR%\python.exe" -m pip install pandas
"%EMBED_DIR%\python.exe" -m pip install matplotlib
"%EMBED_DIR%\python.exe" -m pip install opencv-python
"%EMBED_DIR%\python.exe" -m pip install scikit-learn
"%EMBED_DIR%\python.exe" -m pip install tensorflow
"%EMBED_DIR%\python.exe" -m pip install flask


:: === 5. CZYSZCZENIE ===
echo [5/5] Usuwanie tymczasowych plików...
del /f /q get-pip.py >nul 2>&1
del /f /q "%EMBED_ZIP%" >nul 2>&1


echo.
echo Gotowe!
pause
