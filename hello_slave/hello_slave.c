/* hello_slave.c

   a hello world type program that receives a series of
   bytes via the Game Boy's link cable, and prints them out
   on screen. */

#include <stdio.h>
#include <gb/gb.h>

#include "bkg_layout.c"
#include "avatar.c"
#include "beta_ascii.c"
#include "chat_bubble.c"
#include "pointer.c"

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
#define BUBBLE_BOTTOM_EDGE 0x0A


/* globals */
volatile UBYTE handshake_ok = 0x00;
volatile UBYTE receiving = 0x00;
volatile UBYTE idling = 0x01;
volatile UINT8 ccount = 0;
char message[MAX_LENGTH] = { 0 };


/* All functions in advance */
void setup_bkg_and_sprite (void);
void receive (void);
void debug_receive (void);
void debug_send (void);
void setup_isr (void);
void sio_isr (void);
void tile_print (char *c, UINT8 startx, UINT8 starty, UINT8 clear);
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
void setup_bkg_and_sprite (void) {
  DISPLAY_OFF;
  SPRITES_8x8;
  /* fill tile table */
  set_bkg_data(0, 95, beta_ascii);
  set_bkg_data(95, 64, avatar);
  set_bkg_data(159, 11, chat_bubble)
  set_sprite_data(0, 1, pointer);

  /* establish screen */
  set_sprite_tile(0, 0);
  set_bkg_tiles(0, 0, 20, 18, tweetboy_bkg);
  SHOW_BKG;
  SHOW_SPRITES;
  DISPLAY_ON;
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
    tile_print(message, CRS_START_X, CRS_START_Y, 1);
    idling = 0x01;
    ccount = 0;
  }
}

/*
prints out characters as font tiles on screen, (David J, modified by David H)

param c: char array that should be converted to tiles and printed out
param startx: starting x coordinate for printout
param starty: starting y coordinate for printout
param clear: specifies whether previous printouts should be cleared or not (true:false 1:0)

return: void
*/
void tile_print (char *c, UINT8 startx, UINT8 starty, UINT8 clear) {
  UINT8 ccount;
  UINT8 pos_x = CRS_START_X;
  UINT8 pos_y = CRS_START_Y;
  char ascii_temp[1] = { 0 };

  /* clear previous printout */
  if (clear) {
    for (ccount = 0; ccount < MAX_LENGTH; ccount++) {
      ascii_temp[0] = 0x00;
      set_bkg_tiles (pos_x, pos_y, 1, 1, ascii_temp);
      /* adjust position, end printout if exceeding bubble perimiter */
      if (pos_x == BUBBLE_RIGHT_EDGE) {
        pos_x = CRS_START_X;
        if (pos_y == BUBBLE_BOTTOM_EDGE) {
          break;
        }
        pos_y++;
      } else {
        pos_x++;
      }
    }
  }

  pos_x = startx;
  pos_y = starty;
  for (ccount = 0; ccount < MAX_LENGTH; ccount++) {
    if (c[ccount] == ETB) {
      break;
    }
    ascii_temp[0] = (c[ccount] - ASCII_OFFSET);
    set_bkg_tiles (pos_x, pos_y, 1, 1, ascii_temp);
    /* adjust position */
    if (pos_x == BUBBLE_RIGHT_EDGE) {
      pos_x = CRS_START_X;
      pos_y++;
    } else {
      pos_x++;
    }
    delay(50);
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
  setup_bkg_and_sprite();

  /* keep program waiting for interrupts */
  while (1) {
    waitpad(J_A);
    waitpadup();
    handshake_ok = 0x01;

    while (receiving) {;}
  }

  return 0;
}
