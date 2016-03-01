/* main.c
   
   this uses the chipkit as master to send characters to a gameboy
   via SPI */

#include <stdint.h>
#include <pic32mx.h>

uint8_t message[3] = {0x53,0x4F,0x53};
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
  PORTF ^= 0x1;
  SPI2BUF = c;
  while(!(SPI2STAT & 0x01));
  clrBuf = SPI2BUF;
}

void sleep (void) {
  int i;
  for (i = 0; i < 8000000; i++) {
    /* do nothing */
  }
}

int main (void) {
  uint8_t i;
  char c;

  spi_init();
  TRISF &= ~0x1;
  PORTF = 0x1;

  while (1) {
    c = message[i];
    SPI2BUF = c;
    send_byte();
    if ((IFS(1)&(1<<6))) {
      if (i >= 3) {
        i = 0;
      } else {
        i++;
      }
      IFS(1) = 0;
    }
  }
  return 0;
}
