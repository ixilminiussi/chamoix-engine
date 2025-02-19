@echo off
setlocal enabledelayedexpansion

:: List of shader filenames to compile
set SHADERS=shaded.vert shaded.frag mesh.vert mesh.frag billboard.vert billboard.frag hud.vert hud.frag

:: Source and output directories (update these paths as needed)
set "SOURCE_DIR=..\shaders"
set "OUTPUT_DIR=shaders"

:: Ensure the output directory exists
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

:: Function to compile a shader
for %%S in (%SHADERS%) do (
    set "INPUT_PATH=%SOURCE_DIR%\%%S"
    set "OUTPUT_PATH=%OUTPUT_DIR%\%%S.spv"

    if exist "!INPUT_PATH!" (
        echo Compiling %%S...
        glslc "!INPUT_PATH!" -o "!OUTPUT_PATH!"
        if !ERRORLEVEL! == 0 (
            echo Compiled %%S to !OUTPUT_PATH!
        ) else (
            echo Error compiling %%S
        )
    ) else (
        echo Shader file %%S not found in %SOURCE_DIR%
    )
)

echo Shader compilation complete.

:: Copying cmx assets to build directory
set "SOURCE_DIR=..\assets"
set "OUTPUT_DIR=assets\cmx"

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

xcopy /E /I /Y "%SOURCE_DIR%\*" "%OUTPUT_DIR%"
echo Cmx assets copied.

:: Copying game assets to build directory
set "SOURCE_DIR=..\%1"
set "OUTPUT_DIR=."

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

xcopy /E /I /Y "%SOURCE_DIR%\assets" "%OUTPUT_DIR%"
echo Assets copied.

xcopy /E /I /Y "%SOURCE_DIR%\scenes" "%OUTPUT_DIR%"
echo Scenes copied.

xcopy /E /I /Y "%SOURCE_DIR%\properties" "%OUTPUT_DIR%"
echo Properties copied.

:: Copying editor to the build directory
set "SOURCE_DIR=..\editor"
set "OUTPUT_DIR=editor"

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

xcopy /E /I /Y "%SOURCE_DIR%\*" "%OUTPUT_DIR%"
echo Editor assets copied.

echo Script execution complete.
pause

