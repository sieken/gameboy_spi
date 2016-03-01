/* main.c

   this uses the chipkit as master to send characters to a gameboy
   via SPI */

#include <stdint.h>
#include <pic32mx.h>

#define SLEEP 2000000

// uint8_t message[3] = {0x53,0x4F,0x53};
char message[] = {
  'S','o','c',0x0D,'l','i','s','t','e','r',' ','h','e','m',' ',0x0D,
  'u','r',' ','h','u','s','e',' ','h','a',' ','h','a',' ',0x0D,
  'l','o','l',',','t','r','o','d','d','e',' ','d','u',0x0D,
  'j','a','.',' ','*','F','i','s','!',' ',' ',' ',0x0D,
};
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
    PORTF ^= 0x1;
    sleep();
    i++;
    if (i > 58)
      i = 0;
  }
  return 0;
}
