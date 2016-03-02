/* hello_slave.c

   a hello world type program that receives a series of
   bytes via the Game Boy's link cable, and prints them out
   on screen. */

#include <stdio.h>
#include <gb/gb.h>

char message[19] = { 0 };

#define TR_SIZE   (sizeof(message)/sizeof(message[0]))
#define SC        *(volatile UBYTE *) 0xFF02
#define CR        0x0D

void debug_receive (void) {
  if (_io_status == IO_IDLE) {
    printf("Received: %c\n",_io_in);
  } else {
    printf("Error: %c\n",_io_status);
  }
}

int main (void) {
  UBYTE ccount = 0;
  UBYTE c = 0xAA;
  char input;

  /* don't start receiving until ready */
  printf("Incoming transmission...\n");
  joypad();
  waitpadup();

  /* main routine */
  while (1) {
    _io_out = c;      // write c to gb serial out buffer
    receive_byte();

    /* wait for receive done */
    while (_io_status == IO_RECEIVING) {;}
    input = ((char)_io_in);

    /* check for carriage return */
    if (input == (char)CR ) {
      /* print & clear */
      for (ccount = 0; ccount < TR_SIZE; ccount++) {
        printf("%c",message[ccount]);
        message[ccount] = 0;
      }
      printf("\n");
      ccount = 0;
    } else {
      message[ccount] = input;
    }
    ccount++;
  }

  return 0;
}
