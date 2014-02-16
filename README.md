chip-8-vm
=========
## Dependencies
SDL2

## Usage
`chip-8-vm <chip8 rom file>`

## Somethings to note
I use SDL for graphics rendering and keyboard input.


Compile with **DEBUGGER** defined in your preprocessor to compile with the pseudo-debugger functionality.
Compile with **LOGGING** defined in your preprocessor to compile with the logging functionality.

To use the psuedo-debugger you use load a rom as you normally would, except instructions won't be executed
unless enter is pressed, then each instruction will be printed to the console in Chip-8 assembly language
and an evaluation of the statement where necessary.

It's not recommended to compile with LOGGING defined unless you also compile with DEBUGGER defined because 
it will quickly fill the dump.log file as it dumps the virtual processor state before every instruction is
executed.

## License
Licensed under the [zlib](http://www.zlib.net/zlib_license.html) license.

