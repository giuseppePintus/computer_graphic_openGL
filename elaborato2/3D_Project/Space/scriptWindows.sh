@echo off

:: List of required libraries
set LIBS=mingw-w64-x86_64-gcc mingw-w64-x86_64-glew mingw-w64-x86_64-freeglut mingw-w64-x86_64-assimp

:: Verify presence of required libraries
for %%i in (%LIBS%) do (
    where %%i >nul || (
        echo Installing %%i...
        winget install --id=%%i
    )
)

:: Compile the code using the provided makefile
mingw32-make.exe

:: Run the resulting executable
.\exe.exe
