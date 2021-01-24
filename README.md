# SHS Hommes/Machines Project

This project is carried out within the framework of the SHS Men/Machines course.

``
git submodule update --init --recursive
``

## Compilation

Copy `config.bat.model` to `config.bat` and edit it to specify SFML 5.1 location. You can also specify which of the available pipeline you want to use (GCC or VS). Note that for a compilation with VS pipeline, you should have run `vcvarsall.bat x64`.

To compile, simply run `build.bat` script. 

## Dependencies

* SFML 5.1 x64
