# Hello SPI!

This is a "hello world"-type project, for a Game Boy DMG to connect to a Microchip Uno32 via SPI.

## Compiling & installing

### Uno32

Install the mcb32 environment (found on github) by following provided instructions.

Enter the mcb32 environment and in the hello\_spi directory, run;
´$ make´ and ´$ make install´ to flash it to the Uno32 board.

### Game Boy

Install the GBDK environment (sourceforge or github) by following provided instructions.

In the hello\_slave directory, run;
´$ lcc hello\_slave.c -o hello\_slave.gb´
or whatever you wish to name your output .gb-image.


### TODO

- [ ] Links to sourceforge and github
- [ ] Credit h4xxel and others
