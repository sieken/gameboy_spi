/* main.c
  This file is written by David H (modified by David J)
  This uses the chipkit as master to send characters to a gameboy
  via SPI. The string message[] must be null terminated.
  Implements the use of special handshake codes */

/* mcb32 standard library */
#include <stdint.h>
#include <pic32mx.h>

/* LEDs and sleep defines */
#define LED_NORMAL  2000000
#define ID_SLEEP    1000000
#define TR_SLEEP    25000

/* ASCII defines for readability */
#define EOT   0x04
#define ENQ   0x05
#define ACK   0x06
#define CAN   0x18
#define RMODE 0x01

/* This string is sent to the gameboy */
char message[] = "Hello, Game Boy!";
uint8_t message_length = (sizeof(message)/sizeof(char));

/* globals */
volatile uint8_t clrBuf;
volatile uint8_t idling = 0x01; // starts in idle state
volatile uint8_t active = 0x00;
volatile uint8_t receiving = 0x00;
volatile uint8_t switch_mode = 0x00;
volatile int LED_rate = LED_NORMAL;

/* all functions in advance */
void init (void);
void user_isr (void);
void send (char c);
void sleep (int t);


/* initialization routine */
void init (void) {
  /* turn spi off and clear interrupts */
  SPI2CON = 0;
  IEC(1) = 0;
  IPC(7) = 0;
  IFS(1) = 0;

  /* set interrupts */
  IPC(7) = (7<<24);
  IEC(1) = (1<<7);

  /* set spi */
  SPI2BRG = 0x1FF;      // Set SCK ~78kHz
  SPI2STATCLR = 0x40;   // Clear SPIROV bit
  SPI2CON = 0x8060;     // Set ON, CKE = 0, CKP = 1, MSTEN = 1
  asm("ei");
}

void user_isr (void) {
  clrBuf = SPI2BUF;

  /* ignore CANs and clear clrBuf when switching mode */
  if (receiving && switch_mode) {
    if (clrBuf == CAN)
      clrBuf == EOT;
    else
      switch_mode = 0x00;
  }

  /* receive mode and receive cancel: switch mode to idle mode */
  if (receiving && !switch_mode && clrBuf == CAN) {
    switch_mode = 0x01;
    receiving = 0x00;
    idling = 0x01;
  }

  /* normal receiving, changes LED rate*/
  if (receiving && !switch_mode) {
    if (clrBuf != 1 || clrBuf != 4) { // controls for illegal values
      clrBuf = 1;
    }
    LED_rate = (LED_NORMAL/clrBuf);
  }

  /* discards CANs when switching mode from idling to receiving */
  if (idling && switch_mode) {
    if (clrBuf == CAN)
      clrBuf = EOT;
    else
      switch_mode = 0x00;
  }

  /* receive ACK in idle mode switches to active mode (sending string) */
  if (idling && !switch_mode && clrBuf == ACK) {
    idling = 0x00;
    active = 0x01;
  }

  /* switch from idle mode to receive mode */
  if (idling && !switch_mode && clrBuf == CAN) {
    idling = 0x00;
    receiving = 0x01;
    switch_mode = 0x01;
  }

  IFSCLR(1) = (1<<7);
}

/* sends one character via SPI
 written by F Lundevall, modified by David H */
void send (char c) {
  while(!(SPI2STAT & 0x08));
  SPI2BUF = c;
}

/* bad sleep function */
void sleep (int t) {
  int i;
  for (i = 0; i < t; i++) {/* do nothing */}
}

int main (void) {
  uint8_t ccount = 0x00;

  /* initialize SPI & LED5 */
  init();
  TRISF &= ~0x01;
  PORTF = 0x00;

  /* main routine */
  while (1) {

    /* LED flashes according to LED-rate received from gameboy */
    while (receiving) {
      PORTF ^= 0x01;
      send(RMODE);    // confirms current mode is receive mode
      sleep(LED_rate);
    }

    /* LED off while idling */
    while (idling) {
      PORTF = 0x00;
      send(ENQ);      // confirms mode is idle mode
      sleep(ID_SLEEP);
    }

    /* LED on while sending string */
    while (active) {
      PORTF = 0x01;
      for (ccount = 0; ccount < message_length; ccount++) {
        send(message[ccount]);
        sleep(TR_SLEEP);
      }
      active = 0x00;
      idling = 0x01;
    }
  }
  return 0;
}
