/* hello_slave.c

   a hello world type program that receives a series of
   bytes via the Game Boy's link cable, and prints them out
   on screen. */

#include <stdio.h>
#include <stdlib.h>
#include <gb/gb.h>

#include "tiles.c"

/* interrupt flag, serial control and serial buffer registers */
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
#define CRS_START_X 0x02
#define CRS_START_Y 0x3
#define ASCII_OFFSET 0x20
#define TILE_STEP 0x01
#define BUBBLE_RIGHT_EDGE 0x13

/*
BKG table
tl:  sym   tl:  sym  tl:  sym  tl:  sym  tl:  sym  tl:   sym tl:   sym
------------------------------------------------------------------
0:   (spc) 21:  5    42:  J    63:  _    84:  t    105:  .   126:  .
1:   !     22:  6    43:  K    64:  `    85:  u    106:  .   127:  tl
2:   "     23:  7    44:  L    65:  a    86:  v    107:  .   128:  top
3:   #     24:  8    45:  M    66:  b    87:  w    108:  .   129:  tr
4:   $     25:  9    46:  N    67:  c    88:  x    109:  .   130:  right
5:   %     26:  :    47:  O    68:  d    89:  y    110:  .   131:  lr
6:   &     27:  ;    48:  P    69:  e    90:  z    111:  .   132:  bot
7:   '     28:  <    49:  Q    70:  f    91:  {    112:  .   133:  ll
8.   (     29:  =    50:  R    71:  g    92:  |    113:  .   134:  left
9:   )     30:  >    51:  S    72:  h    93:  }    114:  .   135:  .
10:  *     31:  ?    52:  T    73:  i    94:  ~    115:  .   136:  .
11:  +     32:  @    53:  U    74:  j    95:  guy  116:  .   137:  .
12:  ,     33:  A    54:  V    75:  k    96:  guy  117:  .   138:  bubble
13:  -     34:  B    55:  W    76:  l    97:  guy  118:  .   139:
14:  .     35:  C    56:  X    77:  m    98:  ...  119:  .   140:
15:  /     36:  D    57:  Y    78:  n    99:  .    120:  .   141:
16:  0     37:  E    58:  Z    79:  o    100: .    121:  .   142:
17:  1     38:  F    59:  [    80:  p    101: .    122:  .   143:
18:  2     39:  G    60:  \    81:  q    102: .    123:  .   144:
19:  3     40:  H    61:  ]    82:  r    103: .    124:  .   145:
20:  4     41:  I    62:  ^    83:  s    104: .    125:  .   146:
14

/*
pixel chatbubble distribution
    -----------------------------------------------x
    frm (8 ; 24)              (152 ; 24) frm  row 2
                                                .
    frm (8 ; 80)              (152 ; 80) frm  row 10
col  9     10                      17     20
*/
/* placement (in tiles) of bkg_data on screen
   18 rows of 20 tiles each, numbers representing tile number
   in bkg_data table (see local bkg_init() and gb.h set_bkg_data()) */
const unsigned char tweetboy_bkg[] = {
127,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,129,  //ind 0-19
134,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,130,  //ind 20-39
134,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,130,  //ind 40-59
134,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,130,  //ind 60-79
134,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,130,  //ind 80-99
134,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,130,  //ind 100-119
134,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,130,  //ind 120-139
134,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,130,  //ind 140-159
134,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,130,  //ind 160-179
133,132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,131,  //ind 180-199

 95, 96, 97, 98, 98, 97, 96, 95,  0,135,137,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //ind 200-219
 99,100,101,102,102,101,100, 99,  0,136,138,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //ind 220-239
103,104,105,106,106,105,104,103,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //ind 240-259
107,108,109,110,110,109,108,107,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //ind 260-279
111,112,113,114,114,113,112,111,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //ind 280-299
115,116,117,118,118,117,116,115,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //ind 300-319
119,120,121,122,122,121,120,119,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //ind 320-339
123,124,125,126,126,125,124,123,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  //ind 340-359
};
/*
 01, 02, 03, 04, 05, 06, 07, 08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20*/


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

/* set up basic background */
void setup_bkg (void) {
  disable_interrupts ();
  DISPLAY_OFF;
  /* fill tile table */
  set_bkg_data (0, 95, beta_ascii);
  set_bkg_data (95, 45, guy_n_bubble);

  /* establish screen */
  set_bkg_tiles (0, 0, 20, 18, tweetboy_bkg);
  DISPLAY_ON;
  enable_interrupts ();
}

void sio_isr (void) {
  volatile UBYTE rcv = 0x00;
  UINT8 pos_x = CRS_START_X;
  UINT8 pos_y = CRS_START_Y;
  char ascii_temp[1] = {0x00};
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
      message[ccount] = (char)0x00;
    }
    receiving = 0x00;
  }

  if (!idling && !receiving) {
    for (ccount = 0; ccount < MAX_LENGTH; ccount++) {
      if (message[ccount] == 0x80) {
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
    handshake_ok = 0x00;
    ccount = 0;
  }
  IFLAGS &= ~0x08;
}

void idle_mode (void) {
  while (1) {
    if (joypad()&J_A) {
      waitpadup();
      handshake_ok = 0x01;
    }
  }
}



int main (void) {
  UINT8 i = 0;

  /* don't start receiving until ready */
  SB = 0x00;
  disable_interrupts();
  setup_isr();
  setup_bkg();

  /* main routine */
  while (1) {
    idle_mode();

    /* wait until receive done */
    while (receiving) {;}
  }

  return 0;
}
