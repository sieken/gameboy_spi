/* hello_slave.c

   a hello world type program that receives a series of
   bytes via the Game Boy's link cable, and prints them out
   on screen. */

#include <stdio.h>
#include <gb/gb.h>

#include "bkg_layout.c"
#include "tiles.c"

/* interrupt flag, serial control and serial buffer registers */
#define IEREG     *(volatile UBYTE *) 0xFFFF
#define IFLAGS    *(volatile UBYTE *) 0xFF0F
#define SC        *(volatile UBYTE *) 0xFF02
#define SB        *(volatile UBYTE *) 0xFF01

/* define ascii tokens for readability */
#define ETB 0x00
#define ENQ 0x05
#define ACK 0x06
#define CAN 0x18
#define MAX_LENGTH  144

/* graphic defines */
#define CRS_START_X 0x01
#define CRS_START_Y 0x01
#define ASCII_OFFSET 0x20
#define TILE_STEP 0x01
#define BUBBLE_RIGHT_EDGE 0x12


/* globals */
volatile UBYTE handshake_ok = 0x00;
volatile UBYTE receiving = 0x00;
volatile UBYTE idling = 0x01;
volatile UINT8 ccount = 0;
char message[MAX_LENGTH] = { 0 };


/* All functions in advance */
void setup_bkg (void);
void receive (void);
void debug_receive (void);
void debug_send (void);
void setup_isr (void);
void sio_isr (void);
void idle_mode (void);


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

/* set up basic background (David J) */
void setup_bkg (void) {
  DISPLAY_OFF;
  /* fill tile table */
  set_bkg_data(0, 95, beta_ascii);
  set_bkg_data(95, 45, guy_n_bubble);

  /* establish screen */
  set_bkg_tiles(0, 0, 20, 18, tweetboy_bkg);
  SHOW_BKG;
  DISPLAY_ON;
}

void sio_isr (void) {
  volatile UBYTE rcv = 0x00;
  UINT8 pos_x;
  UINT8 pos_y;
  char ascii_temp[1] = { 0 };
  rcv = SB;

  /* idling & handshaking */
  if (idling) {
    if (rcv == ENQ && handshake_ok) {
      SB = ACK;
      receiving = 0x01;
      idling = 0x00;
      handshake_ok = 0x00;
    }
  }

  /* receiving */
  if (receiving && (rcv == ENQ || rcv == ACK)) {
    /* discard ENQs and ACKs while receiving */
  } else if (receiving && rcv != ETB) {
    message[ccount] = (char)rcv;
    ccount++;
  } else if (receiving && (ccount == MAX_LENGTH || rcv == ETB)) {
    for (; ccount < MAX_LENGTH; ccount++) {
      message[ccount] = (char)ETB;
    }
    receiving = 0x00;
  }

  /* printing to screen (David J, modified by David H) */
  if (!idling && !receiving) {
    pos_x = CRS_START_X;
    pos_y = CRS_START_Y;

    /* clear */
    for (ccount = 0; ccount < MAX_LENGTH; ccount++) {
      ascii_temp[0] = 0x00;
      set_bkg_tiles (pos_x, pos_y, 1, 1, ascii_temp);
      /* adjust position */
      if (pos_x == BUBBLE_RIGHT_EDGE) {
        pos_x = CRS_START_X;
        pos_y++;
      } else {
        pos_x++;
      }
    }

    pos_x = CRS_START_X;
    pos_y = CRS_START_Y;
    for (ccount = 0; ccount < MAX_LENGTH; ccount++) {
      if (message[ccount] == ETB) {
        break;
      }
      ascii_temp[0] = (message[ccount] - ASCII_OFFSET);
      set_bkg_tiles (pos_x, pos_y, 1, 1, ascii_temp);
      /* adjust position */
      if (pos_x == BUBBLE_RIGHT_EDGE) {
        pos_x = CRS_START_X;
        pos_y++;
      } else {
        pos_x++;
      }
      delay(50);
    }//close for-loop

    ascii_temp[0] = 0x00;
    for (; ccount < MAX_LENGTH; ccount++) {
      set_bkg_tiles (pos_x, pos_y, 1, 1, ascii_temp);
      /* adjust position */
      if (pos_x == BUBBLE_RIGHT_EDGE) {
        pos_x = CRS_START_X;
        pos_y++;
      } else {
        pos_x++;
      }
    }//close for-loop
    idling = 0x01;
    ccount = 0;
  }
}

void idle_mode (void) {
  delay(500);
  while (1) {
    if (joypad()&J_A) {
      waitpadup();
      handshake_ok = 0x01;
      break;
    }
  }
}



int main (void) {
  UINT8 i = 0;

  setup_isr();
  setup_bkg();

  /* keep program waiting for interrupts */
  while (1) {
    waitpad(J_A);
    waitpadup();
    handshake_ok = 0x01;

    while (receiving) {;}
  }

  return 0;
}
