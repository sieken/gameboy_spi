/* main.c

   this uses the chipkit as master to send characters to a gameboy
   via SPI */

#include <stdint.h>
#include <pic32mx.h>

#define SLEEP 4000000

// uint8_t message[3] = {0x53,0x4F,0x53};
char message[3] = {'S','O','S'};
uint8_t clrBuf;

void spi_init (void);
void send (char c);
void sleep (void);

void spi_init (void) {
  SPI2CON = 0;
  SPI2BRG = 0x1FF;
  SPI2STATCLR = 0x40;
  SPI2CON = 0x8060;
}

void send (char c) {
  while(!(SPI2STAT & 0x08));
  SPI2BUF = c;
  while(!(SPI2STAT & 0x01));
  clrBuf = SPI2BUF;
}

void sleep (void) {
  int i;
  for (i = 0; i < SLEEP; i++) {
    /* do nothing */
  }
}

int main (void) {
  uint8_t i;
  char c;

  spi_init();
  TRISF &= ~0x1;
  PORTF &= ~0x1;

  while (1) {
    c = message[i];
    send (c);
    sleep();
    if ((IFS(1)&(1<<7))) {
      if (i >= 2) {
        i = 0;
        PORTF ^= 0x1;
      } else {
        i++;
      }
      IFS(1) = 0;
    }
  }
  return 0;
}
