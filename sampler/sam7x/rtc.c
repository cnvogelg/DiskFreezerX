#include "rtc.h"
#include "spi_low.h"
#include "spi.h"
#include "util.h"

#define DS3234_CTRL             0x0e
#define DS3234_STATUS           0x0f
#define DS3234_ADDR             0x18
#define DS3234_DATA             0x19

#define DS3234_STATUS_BSY       0x02

static void rtc_write(u08 reg, u08 value)
{
  spi_low_enable_cs(SPI_RTC_CS_MASK);
  spi_io(0x80 | reg);
  spi_io(value);
  spi_low_disable_cs(SPI_RTC_CS_MASK);
}

u08 rtc_read(u08 reg)
{
  spi_low_enable_cs(SPI_RTC_CS_MASK);
  spi_io(reg);
  u08 result = spi_io(0x00);
  spi_low_disable_cs(SPI_RTC_CS_MASK);
  return result;
}

void rtc_init(void)
{
  spi_low_init_channel(SPI_RTC_CHANNEL,48,0,1); // NCPHA, 48/48=1 MHz -> Clock for RTC
  spi_low_set_channel(SPI_RTC_CHANNEL);

  spi_low_enable();

  // setup control register
  rtc_write(DS3234_CTRL, 0x60);

  spi_low_disable();
}

#define TO_BCD(x)       ((x / 10) << 4 | (x % 10))
#define FROM_BCD(x)     ((x >> 4) * 10 + (x & 0xf))

void rtc_set(rtc_time time)
{
  spi_low_set_channel(SPI_RTC_CHANNEL);
  spi_low_enable();

  for(int i=0;i<7;i++) {
      rtc_write(i, time[i]);
  }

  spi_low_disable();
}

void rtc_get(rtc_time time)
{
  spi_low_set_channel(SPI_RTC_CHANNEL);
  spi_low_enable();

  for(int i=0;i<7;i++) {
      time[i] = rtc_read(i);
  }

  spi_low_disable();
}

void rtc_set_entry(u08 index, u08 value)
{
  spi_low_set_channel(SPI_RTC_CHANNEL);
  spi_low_enable();

  rtc_write(index, value);

  spi_low_disable();
}

u08  rtc_get_entry(u08 index)
{
  spi_low_set_channel(SPI_RTC_CHANNEL);
  spi_low_enable();

  u08 value = rtc_read(index);

  spi_low_disable();
  return value;
}

//                       01234567890123456
static char *time_str = "dd.mm.yy hh:mm:ss";

char *rtc_get_time_str(void)
{
  rtc_time t;
  rtc_get(t);

  byte_to_hex(t[RTC_INDEX_DAY], (u08 *)&time_str[0]);
  byte_to_hex(t[RTC_INDEX_MONTH], (u08 *)&time_str[3]);
  byte_to_hex(t[RTC_INDEX_YEAR], (u08 *)&time_str[6]);

  byte_to_hex(t[RTC_INDEX_HOUR], (u08 *)&time_str[9]);
  byte_to_hex(t[RTC_INDEX_MINUTE], (u08 *)&time_str[12]);
  byte_to_hex(t[RTC_INDEX_SECOND], (u08 *)&time_str[15]);

  return time_str;
}

// RTC SRAM access

void rtc_write_sram(u08 addr, const u08 *data, u08 size)
{
  spi_low_set_channel(SPI_RTC_CHANNEL);
  spi_low_enable();

  rtc_write(DS3234_ADDR, addr);
  for(u08 i=0;i<size;i++) {
      rtc_write(DS3234_DATA, data[i]);
  }

  spi_low_disable();
}

void rtc_read_sram(u08 addr, u08 *data, u08 size)
{
  spi_low_set_channel(SPI_RTC_CHANNEL);
  spi_low_enable();

  rtc_write(DS3234_ADDR, addr);
  for(u08 i=0;i<size;i++) {
      data[i] = rtc_read(DS3234_DATA);
  }

  spi_low_disable();
}
