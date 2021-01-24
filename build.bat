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

ctags -e -R %SOURCES_DIR%

pushd %BUILD_DIR%

if %BUILD_PIPELINE% == %WIN_VS_PIPELINE% (

echo VS Pipeline:

cl /std:c++14 /Od /DDEBUG /Zi /EHsc /I "%INC_SFML%" ..\%SOURCES_DIR%main.cpp ^
   /link ^
   /LIBPATH:"%LIB_SFML%"^
   sfml-main.lib^
   sfml-graphics.lib^
   sfml-window.lib^
   sfml-system.lib^
   sfml-audio.lib^
   /out:main.exe
   
   REM Pour enlever le terminal
   REM /SUBSYSTEM:WINDOWS ^
   
   
) else if %BUILD_PIPELINE% == %WIN_GCC_PIPELINE% (
    
    echo GCC Pipeline:
    
    g++ -c -g -D DEBUG ..\%SOURCES_DIR%main.cpp -I%INC_SFML%
    g++ main.o -o main.exe -L%LIB_SFML% -lsfml-main -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
    
)
   
popd
