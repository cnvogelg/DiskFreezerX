// rtc.h
// Real Time Clock Handling
// (SPI RTC DS3234)

#ifndef RTC_H
#define RTC_H

#include "target.h"

#define RTC_INDEX_SECOND   0
#define RTC_INDEX_MINUTE   1
#define RTC_INDEX_HOUR     2
#define RTC_INDEX_DAY      4
#define RTC_INDEX_MONTH    5
#define RTC_INDEX_YEAR     6

typedef u08 rtc_time[7];

extern void rtc_init(void);

extern void rtc_set(rtc_time time);
extern void rtc_get(rtc_time time);

extern void rtc_set_entry(u08 index, u08 value);
extern u08  rtc_get_entry(u08 index);

extern char *rtc_get_time_str(void);

#endif
