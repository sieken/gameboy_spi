/* main.c

   This uses the chipkit as master to send characters to a gameboy
   via SPI. The string message[] must be null terminated. */

#include <stdint.h>
#include <pic32mx.h>


#define ID_SLEEP    1000000
#define TR_SLEEP    25000

#define EOT   0x04
#define ENQ   0x05
#define ACK   0x06
#define CAN   0x18

volatile uint8_t clrBuf;
volatile uint8_t idling = 0x01; // starts in idle state
volatile uint8_t active = 0x00;

char message[] = "satan lever!";
uint8_t message_length = (sizeof(message)/sizeof(char));
char idle_send = ENQ;

void init (void);
void user_isr (void);
void send_rcv (char c);
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
  SPI2STATCLR = 0x40;
  SPI2CON = 0x8060;     // Set ON, CKE, MSTEN
  asm("ei");
}

void user_isr (void) {
  clrBuf = SPI2BUF;

  /* acknowledges in idle */
  if (idling && clrBuf == ACK) {
    idling = 0x00;
    active = 0x01;
  }

  /* should cancel and revert to idle mode */
  if (clrBuf == CAN) {
    active = 0x00;
    idling = 0x01;
  }

  IFSCLR(1) = (1<<7);
}

/* sends one character via SPI */
void send_rcv (char c) {
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
  PORTF = 0x01;

  /* main routine */
  while (1) {
    PORTF = 0x00;
    while (idling) {
      send_rcv(idle_send);
      sleep(ID_SLEEP);
    }

    while (active) {
      PORTF = 0x01;
      for (ccount = 0; ccount < message_length; ccount++) {
        send_rcv(message[ccount]);
        sleep(TR_SLEEP);
      }
      active = 0x00;
      idling = 0x01;
    }
  }

  return 0;
}
