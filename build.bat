@echo off

set BUILD_DIR=build\
set SOURCES_DIR=sources\

REM If the config.bat file does not exist, copy it from model
if not exist config.bat (
    copy config.bat.model config.bat
    echo ---
    echo Config script has been copied but should be manualy complete!
    echo ---
)
call config.bat

if not exist %BUILD_DIR% (
   mkdir %BUILD_DIR%
   copy %DLL_SFML%\*.dll %BUILD_DIR%
)

pushd %BUILD_DIR%

if %BUILD_PIPELINE% == %WIN_VS_PIPELINE% (

echo VS Pipeline:

REM /P for output preprocessor

REM set /DDEBUG=0 for avoid compilation of debug code

cl /std:c++14 /Od /DDEBUG=0 /Zi /EHsc ^
   /I ..\sources\includes ^
   /I "%INC_SFML%" ^
   /I ..\vendors\imgui ^
   /I ..\vendors\imgui-sfml ^
   ..\%SOURCES_DIR%main.cpp ^
   /link ^
   Opengl32.lib^
   /LIBPATH:"%LIB_SFML%"^
   sfml-main.lib^
   sfml-graphics.lib^
   sfml-window.lib^
   sfml-system.lib^
   sfml-audio.lib^
   /SUBSYSTEM:WINDOWS^	
   /out:main.exe

   
   REM Pour enlever le terminal
   REM /SUBSYSTEM:WINDOWS ^
   
   
) else if %BUILD_PIPELINE% == %WIN_GCC_PIPELINE% (
    
    echo GCC Pipeline:

	REM -E pour afficher la sortie du preprocessor
    g++ -w -g -DDEBUG=0 -c ..\%SOURCES_DIR%main.cpp -o main.o -I..\sources\includes -I%INC_SFML% -I..\vendors\imgui -I..\vendors\imgui-sfml
	g++ main.o -o main.exe -lOpengl32 -L%LIB_SFML% -lsfml-main -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
    
)
   
popd
