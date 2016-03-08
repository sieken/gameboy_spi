/* hello_slave.c

   This file written by David H (Modified by David J).
   a hello world type program that receives a series of
   bytes via the Game Boy's link cable, and prints them out
   on screen. Also allows a send mode, where information gets sent
   via the link cable.
   Operates mainly on interrupts, and requires specific 'handshake' codes to function properly */

/* GBDK standard library */
#include <stdio.h>
#include <gb/gb.h>

/* Tiles and sprites library */
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
#define RMODE 0x01
#define MAX_LENGTH  144

/* graphic defines */
#define CRS_START_X 0x01
#define CRS_START_Y 0x01
#define POINTER_ACTIVE_Y 4
#define POINTER_ACTIVE_X 2
#define TILE_STEP 0x01
#define POINTER_STEP 3
#define ASCII_OFFSET 0x20
#define BUBBLE_RIGHT_EDGE 0x12
#define BUBBLE_BOTTOM_EDGE 0x0A



/* globals */
volatile UBYTE handshake_ok = 0x00;
volatile UBYTE switch_mode = 0x00;
volatile UBYTE receiving = 0x00;
volatile UBYTE idling = 0x01;
volatile UBYTE sending = 0x00;
volatile UINT8 ccount = 0;
volatile UINT8 LED_rate = 1;
char message[MAX_LENGTH] = { 0 };


/* All functions in advance */
void receive (void);
void debug_receive (void);
void debug_send (void);
void setup_isr (void);
void setup_bkg_and_sprite (void);
void sio_isr (void);
void tile_print (char *c, UINT8 startx, UINT8 starty, UINT8 clear);
void setup_b_mode (void);
UINT8 move_pointer (UINT8 point_y);



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

/* Sets up the interrupts for serial IO */
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
  /* fill tile table */
  set_bkg_data(0, 95, beta_ascii);
  set_bkg_data(95, 64, avatar);
  set_bkg_data(159, 11, chat_bubble);
  set_bkg_data(171, 1, pointer);

  /* establish screen */
  set_bkg_tiles(0, 0, 20, 18, bkg_layout);
  SHOW_BKG;
  wait_vbl_done();
  DISPLAY_ON;
}

void sio_isr (void) {
  volatile UBYTE rcv = 0x00;
  rcv = SB;
  SB = 0x00;

  /* switching from idle to send mode */
  if (idling && switch_mode) {
    if (rcv == RMODE) {
      switch_mode = 0x00;
      idling = 0x00;
      sending = 0x01;
    } else {
      SB = CAN;
    }
  }

  /* normal idling & handshaking */
  if (idling && !switch_mode) {
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
    ccount = 0x00;
  }

  /* switch from send mode to idle mode */
  if (sending && switch_mode) {
    if (rcv == ENQ) {
      switch_mode = 0x00;
      sending = 0x00;
      idling = 0x01;
    } else {
      SB = CAN;
    }
  }

  /* normal send mode */
  if (sending && !switch_mode) {
    if (rcv == RMODE)
      SB = (UBYTE)(LED_rate);
  }


  /* printing to screen (David J, modified by David H) */
  if (!idling && !receiving && !sending) {
    tile_print(message, CRS_START_X, CRS_START_Y, 1);
    idling = 0x01;
  }
}

/*
   prints out characters as font tiles on screen, (David J, modified by David H)

   param c: char array that should be converted to tiles and printed out
   param startx: starting x coordinate for printout
   param starty: starting y coordinate for printout
   param clear: specifies whether previous printouts should be cleared or not. Only works
   when clearing the bubble dialogue. Set with 1 (true) or 0 (false)

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
        pos_y++;
      } else {
        pos_x++;
      }
    }
  }

  /* print out from parameter coordinates */
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
      if (pos_y == BUBBLE_BOTTOM_EDGE)
        break;
      pos_y++;
    } else {
      pos_x++;
    }
    delay(50);
  }
}

void setup_b_mode (void) {
  tile_print("Press A to select", CRS_START_X, CRS_START_Y, 1);
  tile_print("LED speed: ", CRS_START_X, (CRS_START_Y + 1), 0);
  tile_print("1", 3, 4, 0);
  tile_print("2", 3, 7, 0);
  move_pointer(7);
}

UINT8 move_pointer (UINT8 point_y){
  char clr_pointer[] = {0x00};
  char pointer_temp[] = {171};
  UINT8 next_y = (((point_y - POINTER_ACTIVE_Y)^POINTER_STEP) + POINTER_ACTIVE_Y);
  set_bkg_tiles(2, point_y, 1, 1, clr_pointer);
  set_bkg_tiles(2, next_y, 1, 1, pointer_temp);
  return next_y;
}

int main (void) {
  UINT8 i = 0;
  UINT8 sending_x = 0x12;
  UINT8 sprite_y = 0x04;

  setup_isr();
  setup_bkg_and_sprite();
  tile_print("A: R/S", 12, 12, 0);
  tile_print("B: Mode", 12, 13, 0);

  /* keep program waiting for interrupts */
  while (1) {
    while (sending) {
      switch (joypad()) {
        case J_A:
          waitpadup();
          LED_rate = (UINT8)((sprite_y - POINTER_ACTIVE_Y) + 1);
          break;
        case J_UP:
          waitpadup();
          sprite_y = move_pointer(sprite_y);
          break;
        case J_DOWN:
          waitpadup();
          sprite_y = move_pointer(sprite_y);
          break;
        case J_B:
          waitpadup();
          switch_mode = 0x01;
          sprite_y = 0x00;
          move_sprite(0,0,0);
          tile_print("Press A to refresh", CRS_START_X, CRS_START_Y, 1);
          break;
      }
    }

    while (idling) {
      switch (joypad()) {
        case J_A:
          waitpadup();
          handshake_ok = 0x01;
          break;
        case J_B:
          waitpadup();
          switch_mode = 0x01;
          setup_b_mode();
          break;
      }
    }

    while (receiving) {;}
  }

  return 0;
}
