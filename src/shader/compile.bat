@echo off
setlocal

REM Vérifier si le Vulkan SDK est défini
if "%VULKAN_SDK%"=="" (
    echo Vulkan SDK introuvable !
    exit /b 1
)

set GLSLC=%VULKAN_SDK%\Bin\glslc.exe

REM Vérifier si glslc existe
if not exist "%GLSLC%" (
    echo glslc.exe introuvable !
    exit /b 1
)

REM Créer dossier SPV si absent
if not exist SPV (
    mkdir SPV
)

echo Compiling Ray Tracing Shaders...

REM Boucle sur tous les shaders ray tracing
for %%f in (*.rgen *.rchit *.rmiss) do (
    echo Compilation de %%f
    "%GLSLC%" --target-env=vulkan1.2 %%f -o SPV\%%f.spv
)

echo Done!
endlocal