@echo OFF
SETLOCAL

if exist "C:\msys64\ucrt64\bin" (
    SET "PATH=C:\msys64\ucrt64\bin;%PATH%"
) else (
    where g++ >nul 2>nul
    if %ERRORLEVEL% NEQ 0 (
        echo [ERROR] C++20 Compiler not found!
        echo Please install MSYS2 to the default location (C:\msys64) 
        echo or add your custom compiler 'bin' directory to your Windows Path environment variable.
        exit /b 1
    )
)

cd build

cmake -G "MinGW Makefiles" ..
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed!
    exit /b %ERRORLEVEL%
)

cmake --build . -- -j 8
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation failed!
    exit /b %ERRORLEVEL%
)

echo [SUCCESS] Engine build complete!
ENDLOCAL