#include <stdio.h>
#include <gb/gb.h>

#define SC  *(volatile UBYTE *) 0xFF02
#define CR  0x0D

char transmission[19] = { 0 };

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
  char input;
  printf("Incoming transmission...\n");

  while (1) {
    _io_out = c;
    receive_byte();
    while (_io_status == IO_RECEIVING) {;}

    input = (char) _io_in;
    if (input == (char)CR ) {
      for (i = 0; i < 3; i++) { 
        printf("%c",transmission[i]);
      }
      printf("\n");
      i = 0;
    } else { 
      transmission[i] = input;
    }
    i++;
  }

  return 0;
}
