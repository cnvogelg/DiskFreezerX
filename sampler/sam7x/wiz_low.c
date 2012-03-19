#include "wiz_low.h"
#include "spi_low.h"
#include "spi.h"

#define WIZ_OP_WRITE    0xf0
#define WIZ_OP_READ     0x0f

void wiz_low_init(void)
{
  spi_low_init_channel(SPI_WIZ_CHANNEL,48,1,0); // NCPHA, 48/48=1 MHz -> Clock for RTC
}

void wiz_low_begin(void)
{
  spi_low_set_channel(SPI_WIZ_CHANNEL);
  spi_low_enable();
}

void wiz_low_end(void)
{
  spi_low_disable();
}

void wiz_low_write(u16 addr, u08 value)
{
  spi_low_enable_cs(SPI_WIZ_CS_MASK);
  spi_io(WIZ_OP_WRITE);
  spi_io(addr >> 8);
  spi_io(addr & 0xff);
  spi_io(value);
  spi_low_disable_cs(SPI_WIZ_CS_MASK);
}

u08  wiz_low_read(u16 addr)
{
  spi_low_enable_cs(SPI_WIZ_CS_MASK);
  spi_io(WIZ_OP_READ);
  spi_io(addr >> 8);
  spi_io(addr & 0xff);
  u08 result = spi_io(0);
  spi_low_disable_cs(SPI_WIZ_CS_MASK);
  return result;
}

void wiz_low_write_word(u16 addr, u16 value)
{
  u08 d;
  d = (u08)(value >> 8);
  wiz_low_write(addr,d);
  d = (u08)(value & 0xff);
  wiz_low_write(addr+1,d);
}

u16  wiz_low_read_word(u16 addr)
{
  u08 d;
  d = wiz_low_read(addr);
  u16 result = (u16)d << 8;
  d = wiz_low_read(addr+1);
  result |= (u16)d;
  return result;
}
