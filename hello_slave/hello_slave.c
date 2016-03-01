#include <stdio.h>
#include <gb/gb.h>

#define SC  *(volatile UBYTE *) 0xFF02

int main (void) {
  UBYTE c = 0xAA;
  while (1) {
    if (joypad() & J_START) {
      waitpadup();
      _io_out = c;
      SC = 0x80;
      receive_byte();
      printf("Receiving...\n");
      while (_io_status == IO_RECEIVING) {
        /* wait */
      }
      if (_io_status == IO_IDLE) {
        printf ("Received: %x\n",_io_in);
      } else {
        printf ("Error: %x\n",_io_status);
      }
    }
  }
  return 0;
}
