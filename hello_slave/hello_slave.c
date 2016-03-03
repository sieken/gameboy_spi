/* hello_slave.c

   a hello world type program that receives a series of
   bytes via the Game Boy's link cable, and prints them out
   on screen. */

#include <stdio.h>
#include <gb/gb.h>

/* interrupt flag, serial control and serial buffer registers */
#define IFLAGS    *(volatile UBYTE *) 0xFF0F
#define SC        *(volatile UBYTE *) 0xFF02
#define SB        *(volatile UBYTE *) 0xFF01

/* define ascii tokens for readability */
#define ETB 0x00
#define ENQ 0x05
#define ACK 0x06
#define CAN 0x18

#define MAX_LENGTH  141

/* globals */
volatile UBYTE handshake_ok = 0x00;
volatile UBYTE receiving = 0x00;
volatile UBYTE idling = 0x01;
volatile UINT8 ccount = 0;
char message[MAX_LENGTH] = { 0 };


/* All functions in advance */
void receive (void);
void debug_receive (void);
void debug_send (void);
void setup_isr (void);
void sio_isr (void);
void idle_mode (void);
void print_message (void);





void receive (void) {
  receive_byte();
  /* Wait for IO completion */
  while (_io_status == IO_RECEIVING) {;}
}

void debug_receive (void) {
  printf("DEBUG receive_byte()\n");
  receive();
  if (_io_status == IO_IDLE) {
    printf("Received: %x\n",_io_in);
  } else {
    printf("Error: %x\n",_io_status);
  }
}

void debug_send (void) {
  printf("DEBUG send_byte()\n");
  send_byte();
  while (_io_status == IO_SENDING) {
    /* wait for io completion */
  }
  if (_io_status == IO_IDLE) {
    printf("Byte sent");
  } else {
    printf("Error: %x\n",_io_status);
  }
}

void setup_isr (void) {
  disable_interrupts();
  IFLAGS = (UBYTE)0x00;
  add_SIO(sio_isr);
  enable_interrupts();
  set_interrupts(SIO_IFLAG);
}

void sio_isr (void) {
  volatile UBYTE rcv = 0x00;
  rcv = SB;

  /* idling & handshaking */
  if (idling) {
    if (rcv == ENQ && handshake_ok) {
      SB = ACK;
      receiving = 0x01;
      idling = 0x00;
    }
    printf("idling rcv: %c\n", rcv);
  }

  /* receiving */
  if (receiving && rcv == ENQ) {
    /* discard ENQs and ACKs */
  } else if (receiving && rcv != ETB) {
    message[ccount] = (char)rcv;
    ccount++;
    printf("receiving rcv: %c\n", rcv);
  } else if (receiving && (ccount == MAX_LENGTH || rcv == ETB)) {
    for (; ccount < MAX_LENGTH; ccount++) {
      message[ccount] = (char)0x00;
    }
    SB = CAN;
    receiving = 0x00;
    handshake_ok = 0x00;
    idling = 0x01;
  }
}

void idle_mode (void) {
  while (1) {
    if (joypad()&J_A) {
      waitpadup();
      handshake_ok = 0x01;
    }
  }
}

void print_message (void) {
  UINT8 i;
  for (i = 0; i < MAX_LENGTH; i++) {
    putchar(message[i]);
  }
}


int main (void) {
  /* don't start receiving until ready */
  SB = 0x00;
  disable_interrupts();
  printf("Incoming transmission\n");
  setup_isr();

  /* main routine */
  while (1) {
    idle_mode();

    /* wait until receive done */
    while (receiving) {;}

    print_message();
  }

  return 0;
}
