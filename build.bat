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

cl /std:c++14 /Od /Zi /EHsc /I "%INC_SFML%" ..\%SOURCES_DIR%main.cpp ^
   /link ^
   /SUBSYSTEM:WINDOWS ^
   /LIBPATH:"%LIB_SFML%"^
   sfml-main.lib^
   sfml-graphics.lib^
   sfml-window.lib^
   sfml-system.lib^
   sfml-audio.lib^
   /out:main.exe
   
popd
