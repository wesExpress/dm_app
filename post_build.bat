@echo OFF
SetLocal EnableDelayedExpansion

SET SRC_DIR=%cd%

IF NOT EXIST "build\assets\shaders"  mkdir build\assets\shaders
IF NOT EXIST "build\assets\textures" mkdir build\assets\textures
IF NOT EXIST "build\assets\fonts"    mkdir build\assets\fonts

REM shaders
CD assets/shaders

FOR /R %%f IN (*.glsl) DO (
    SET fname=%%f
    SET root=!fname:~0,-5!
    SET output=!root!.spv
    SET shader_type=!root:~-5!

    ECHO Compiling shader: !fname!
    IF /I "!shader_type!" EQU "pixel" (
        SET shader_flags=-fshader-stage=frag
    ) ELSE IF /I "!shader_type!" EQU "ertex" (
        SET shader_flags=-fshader-stage=vert
    ) ELSE IF /I "!shader_type" EQU "mpute" (
        SET shader_flags=-fshader-stage=vert
    )
    ECHO !shader_flags!

    %VULKAN_SDK%\bin\glslc !shader_flags! !fname! -o !output!
    MOVE !output! %SRC_DIR%\build\assets\shaders
)

FOR /R %%f IN (*.hlsl) DO (
    SET fname=%%f
    SET root=!fname:~0,-5!
    SET output=!root!.cso
    SET shader_type=!root:~-5!
    SET debug_shader=!root!.pdb

    ECHO Compiling shader: !fname!
    IF /I "!shader_type!" EQU "pixel" (
        SET shader_flags=/E main /T ps_6_6
    ) ELSE IF /I "!shader_type!" EQU "ertex" (
        SET shader_flags=/E main /T vs_6_6
    ) ELSE IF /I "!shader_type!" EQU "mpute" (
        SET shader_flags=/E main /T cs_6_6
    )
    ECHO !shader_flags!

    dxc !shader_flags! !fname! /Zi /Fd !debug_shader! /Fo !output!

    MOVE !output! %SRC_DIR%\build\assets\shaders
    MOVE !debug_shader! %SRC_DIR%\build\assets\shaders
)

CD ..\..

COPY /y "assets\textures" build\assets\textures
COPY /y "assets\fonts" build\assets\fonts
