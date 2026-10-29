@echo off
setlocal EnableExtensions EnableDelayedExpansion

pushd "%~dp0"

set "APK_PATH=%~1"
if not defined APK_PATH set "APK_PATH=%CD%\app\build\outputs\apk\debug\app-debug.apk"

if not exist "%APK_PATH%" (
    echo ERROR: APK not found:
    echo   %APK_PATH%
    echo.
    echo Build it first with:
    echo   gradlew.bat assembleDebug
    popd
    exit /b 1
)

set "ADB_EXE="
for /f "delims=" %%I in ('where adb.exe 2^>nul') do if not defined ADB_EXE set "ADB_EXE=%%I"

if not defined ADB_EXE if defined ANDROID_HOME if exist "%ANDROID_HOME%\platform-tools\adb.exe" (
    set "ADB_EXE=%ANDROID_HOME%\platform-tools\adb.exe"
)

if not defined ADB_EXE if defined ANDROID_SDK_ROOT if exist "%ANDROID_SDK_ROOT%\platform-tools\adb.exe" (
    set "ADB_EXE=%ANDROID_SDK_ROOT%\platform-tools\adb.exe"
)

if not defined ADB_EXE if exist "local.properties" (
    for /f "tokens=1,* delims==" %%A in (local.properties) do (
        if /i "%%A"=="sdk.dir" set "SDK_DIR=%%B"
    )
    if defined SDK_DIR if exist "!SDK_DIR!\platform-tools\adb.exe" (
        set "ADB_EXE=!SDK_DIR!\platform-tools\adb.exe"
    )
)

if not defined ADB_EXE (
    echo ERROR: adb.exe was not found.
    echo Install Android SDK Platform-Tools and either add adb to PATH or set
    echo sdk.dir in android\local.properties.
    popd
    exit /b 1
)

set "DEVICE_SERIAL=%~2"
set /a DEVICE_COUNT=0

if not defined DEVICE_SERIAL (
    for /f "skip=1 tokens=1,2" %%A in ('"%ADB_EXE%" devices 2^>nul') do (
        if "%%B"=="device" (
            set /a DEVICE_COUNT+=1
            set "DEVICE_SERIAL=%%A"
        )
    )

    if !DEVICE_COUNT! EQU 0 (
        echo ERROR: No authorized Android device was found.
        echo Connect the phone, enable USB debugging, and accept the authorization prompt.
        "%ADB_EXE%" devices
        popd
        exit /b 1
    )

    if !DEVICE_COUNT! GTR 1 (
        echo ERROR: More than one Android device is connected.
        echo Pass the desired serial number as the second argument:
        echo   install-apk.bat "path\to\app.apk" DEVICE_SERIAL
        "%ADB_EXE%" devices
        popd
        exit /b 1
    )
)

echo Installing:
echo   %APK_PATH%
echo Device:
echo   %DEVICE_SERIAL%
echo.

"%ADB_EXE%" -s "%DEVICE_SERIAL%" install -r "%APK_PATH%"
set "INSTALL_RESULT=%ERRORLEVEL%"

if not "%INSTALL_RESULT%"=="0" (
    echo.
    echo ERROR: adb install failed with exit code %INSTALL_RESULT%.
    popd
    exit /b %INSTALL_RESULT%
)

echo.
echo APK installed successfully.
popd
exit /b 0
