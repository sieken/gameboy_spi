# Hello SPI!

This is a "hello world"-type project, for a Game Boy DMG to connect to a Microchip Uno32 via SPI.

## Compiling & installing

### Uno32

Install the mcb32 environment (found [here](https://github.com/is1200-example-projects/mcb32tools/releases/)) by following provide instructions.

Enter the mcb32 environment and in the hello\_spi directory, run;
`$ make` and `$ make install` to flash it to the Uno32 board.

### Game Boy

Install the GBDK environment from [here](https://sourceforge.net/projects/gbdk/files/) ([this](http://sourceforge.net/projects/gbdk
) fork for OSX works for me) by following provided instructions.
Don't forget to skim through, and fix, the issues reported if you go with the OSX fork.

In the hello\_slave directory, run;
`$ lcc hello_slave.c -o hello_slave.gb`
or whatever you wish to name your output .gb-image.


### TODO

- [x] Links to sourceforge and github
- [ ] Credit h4xxel and others
