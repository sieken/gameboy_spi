/* hello_slave.c

   a hello world type program that receives a series of
   bytes via the Game Boy's link cable, and prints them out
   on screen. */

#include <stdio.h>
#include <gb/gb.h>

char message[19] = { 0 };
volatile UBYTE handshake_ok = 0x00;

#define IFLAGS    *(volatile UBYTE *) 0xFF0F
#define SC        *(volatile UBYTE *) 0xFF02
#define SB        *(volatile UBYTE *) 0xFF01

#define CR        0x0D
#define TR_SIZE   19
#define HANDSHAKE 0xAA

/* All functions in advance */
void receive (void);
void debug_receive (void);
void debug_send (void);
void wait_handshake (void);
void sio_isr (void);
void setup_isr (void);



void receive (void) {
  receive_byte();
  while (_io_status == IO_RECEIVING) {
    /* Wait for IO completion */
  }
}

void debug_receive (void) {
  printf("DEBUGGING receive_byte()\n");
  receive();
  while (_io_status == IO_RECEIVING) {
    /* wait for io completion */
  }
  if (_io_status == IO_IDLE) {
    printf("Received: %x\n",_io_in);
  } else {
    printf("Error: %x\n",_io_status);
  }
}

void debug_send (void) {
  printf("DEBUGGING send_byte()\n");
  while (_io_status == IO_SENDING) {
    /* wait for io completion */
  }
  if (_io_status == IO_IDLE) {
    printf("Byte sent");
  } else {
    printf("Error: %x\n",_io_status);
  }
}

void wait_handshake (void) {
  while (1) {
    if (handshake_ok) {
      break;
    }
  }
}

void sio_isr (void) {
  int handshake;
  handshake = SB;
  if (handshake == 0xAA) {
    handshake_ok = (UBYTE)0x01;
    SB = 0xAA;
  }
  printf("INTERRUPTS FUNKAR!\n");
  printf("RECEIVED: %x : %c\n", handshake, handshake);
}

void setup_isr (void) {
  disable_interrupts();
  IFLAGS = (UBYTE) 0x00;
  add_SIO(sio_isr);
  enable_interrupts();
  set_interrupts(SIO_IFLAG);
}

int main (void) {
//  UBYTE ccount = 0;
//  UBYTE output = 0x00;
//  char input;
  /* don't start receiving until ready */
  disable_interrupts();
  while (1) {
    if (joypad()&J_A) {
      waitpadup();
      break;
    }
  }
  printf("Incoming transmission\n");
  delay(1000);
  setup_isr();
  printf("...\n");

  /* main routine */
  while (1) {}
  //    _io_out = output;      // write c to gb serial out buffer
  //    receive_byte();
  //
  //    /* wait for receive done */
  //    while (_io_status == IO_RECEIVING) {;}
  //    input = ((char)_io_in);
  //
  //    /* check for carriage return */
  //    if (input == (char)CR ) {
  //      /* print & clear */
  //      for (ccount = 0; ccount < TR_SIZE; ccount++) {
  //        printf("%c",message[ccount]);
  //        message[ccount] = 0;
  //      }
  //      printf("\n");
  //      ccount = 0;
  //    } else {
  //      message[ccount] = input;
  //    }
  //    ccount++;
  //  }
  //  return 0;
  }
