/* hello_slave.c

   a hello world type program that receives a series of
   bytes via the Game Boy's link cable, and prints them out
   on screen. */

#include <stdio.h>
#include <gb/gb.h>

char message[19] = { 0 };
UBYTE handshake = 0x00;

#define TR_SIZE   19
#define SC        *(volatile UBYTE *) 0xFF02
#define CR        0x0D

void debug_receive (void) {
  printf("DEBUGGING receive_byte()\n");
  while (_io_status == IO_RECEIVING) {
    /* wait for io completion */
  }
  if (_io_status == IO_IDLE) {
    printf("Received: %c\n",_io_in);
  } else {
    printf("Error: %c\n",_io_status);
  }
}

void debug_send (void) {
  printf("DEBUGGING send_byte()\n");
  while (_io_status == IO_SENDING) {
    /* wait for io completion */
  }
  if (_io_status == IO_IDLE) {
    printf("Byte sent: %c\n");
  } else {
    printf("Error: %c\n",_io_status);
  }
}

/* handshake routine */
void wait_handshake (void) {
  while (1) {
    _io_out = 0;
    if (joypad() & J_A) {
      waitpadup();
      handshake = 0xAA;
      _io_out = handshake;
      receive_byte();
      debug_receive();
    }
  }
}

int main (void) {
  UBYTE ccount = 0;
  UBYTE output = 0x00;
  char input;

  /* don't start receiving until ready */
  printf("Incoming transmission\n");
  wait_handshake();
  printf("...\n");

  /* main routine */
  while (1) {
    _io_out = output;      // write c to gb serial out buffer
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
