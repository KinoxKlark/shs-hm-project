# SHS Hommes/Machines Project

This project is carried out within the framework of the SHS Men/Machines course.

``
git submodule update --init --recursive
``

## Compilation

Copy `config.bat.model` to `config.bat` and edit it to specify SFML 5.1 location. You can also specify which of the available pipeline you want to use (GCC or VS). Note that for a compilation with VS pipeline, you should have run `vcvarsall.bat x64`.

To compile, simply run `build.bat` script. 

### SFML Customization

Headers of SFML has been changed and new versions are in `modified_headers/`. Thoses new headers has to be copied in SFML `include/` directory in order to be able to compile the software.

### Recup script

The script `recup.bat` does a `git pull` followed by a `call build.bat` and can be use for quickly get and compile the project. It should be use once the configuration has been done.

## Dependencies

* SFML 5.1 x64
