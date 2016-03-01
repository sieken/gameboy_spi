#include <stdio.h>
#include <gb/gb.h>

#define SC  *(volatile UBYTE *) 0xFF02

char transmission[3] = { 0 };

void debug_receive (void) {
  if (_io_status == IO_IDLE) {
    printf ("Received: %c\n",_io_in);
  } else {
    printf ("Error: %c\n",_io_status);
  }
}

int main (void) {
  UBYTE i = 0;
  UBYTE c = 0xAA;
  printf("Incoming transmission...\n");

  while (1) {
    _io_out = c;
    receive_byte();
    while (_io_status == IO_RECEIVING) {;}

    transmission[i] = _io_in;
    i++;
    if (i >= 3) {
      for (i = 0; i < 3; i++) {
        printf("%c",transmission[i]);
      }
      printf("\n");
      i = 0;
    }
  }
  return 0;
}
