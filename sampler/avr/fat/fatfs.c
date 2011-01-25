#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "integer.h"
#include "ff.h"
#include "diskio.h"
#include "mmc.h"
#include "fatfs.h"

FATFS fatfs;

extern void fatfs_init(uint8_t clock_div)
{
	// init CS pin: out hi
	SD_CS_DDR  |= _BV(SD_CS_PIN);
	SD_CS_PORT |= _BV(SD_CS_PIN);

	// MISO: in, high
	SD_MISO_DDR  &= ~_BV(SD_MISO_PIN);
	SD_MISO_PORT |=  _BV(SD_MISO_PIN);

	// MOSI: out
	SD_MOSI_DDR  |= _BV(SD_MOSI_PIN);
	SD_CLK_DDR   |= _BV(SD_CLK_PIN);

#if !defined(SD_SOFTWARE_SPI)
  //SS has to be output or input with pull-up
# if (defined(__AVR_ATmega1280__) || \
      defined(__AVR_ATmega1281__) || \
      defined(__AVR_ATmega2560__) || \
      defined(__AVR_ATmega2561__))     //--- Arduino Mega ---
#  define SS_PORTBIT (0) //PB0
# else                                 //--- Arduino Uno ---
#  define SS_PORTBIT (2) //PB2
# endif
  if(!(DDRB & (1<<SS_PORTBIT))) //SS is input
  {
      PORTB |= (1<<SS_PORTBIT); //pull-up on
  }

  //init hardware spi
  switch(clock_div)
  {
    case 2:
      SPCR = (1<<SPE)|(1<<MSTR); //enable SPI, Master, clk=Fcpu/4
      SPSR = (1<<SPI2X); //clk*2 = Fcpu/2
      break;
    case 4:
      SPCR = (1<<SPE)|(1<<MSTR); //enable SPI, Master, clk=Fcpu/4
      SPSR = (0<<SPI2X); //clk*2 = off
      break;
    case 8:
      SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0); //enable SPI, Master, clk=Fcpu/16
      SPSR = (1<<SPI2X); //clk*2 = Fcpu/8
      break;
    case 16:
      SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0); //enable SPI, Master, clk=Fcpu/16
      SPSR = (0<<SPI2X); //clk*2 = off
      break;
    case 32:
      SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1); //enable SPI, Master, clk=Fcpu/64
      SPSR = (1<<SPI2X); //clk*2 = Fcpu/32
      break;
  }
#endif
}

extern uint8_t fatfs_mount(void)
{
  if(disk_initialize(0) & STA_NOINIT)
  {
    return 1;
  }

  if(f_mount(0, &fatfs) == FR_OK)
  {
    return 0;
  }

  return 2;
}

extern void fatfs_umount(void)
{
  f_mount(0, 0);
  disk_ioctl(0, CTRL_POWER, 0); //power off
}
