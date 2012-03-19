/*
 * target.h
 *
 * defines for the target board's hardware
 * including IO pin definitions
 *
 */

#ifndef TARGET_H
#define TARGET_H

#include "board.h"

// ----- I/O Pins 0..31 -----

// two buttons
#define BUTTON1_PIN             23//19
#define BUTTON2_PIN             24//20
// leds
#define LED_GREEN_PIN           18
#define LED_YELLOW_PIN          17

// serial (reserved)
#define SERIAL_RXD_PIN          21
#define SERIAL_TXD_PIN          22

// floppy lines
// OUT
#define WRITE_DATA_PIN          0
#define WRITE_GATE_PIN          31
#define SIDE_SELECT_PIN         29
#define HEAD_STEP_PIN           28
#define DIR_SELECT_PIN          27
#define MOTOR_ENABLE_PIN        19//23
#define DRIVE_SELECT_PIN        10
#define DENSITY_SELECT_PIN      2

// IN
#define READ_DATA_PIN           26
#define TRACK_ZERO_PIN          9
#define INDEX_PIN               30
#define WRITE_PROTECT_PIN       1//new
// SPI (reserved)
#define SPI_MISO_PIN            12
#define SPI_MOSI_PIN            13
#define SPI_CLK_PIN             14

// SD Card
#define SD_SOCKET_WP_PIN        25
#define SD_SOCKET_INS_PIN       15
#define SPI_SD_CS_PIN           11

// SPI RAM
#define SPI_RAM_A0_PIN          5
#define SPI_RAM_A1_PIN          6
#define SPI_RAM_A2_PIN          7
#define SPI_RAM_CS_PIN          8

// RTC
#define SPI_RTC_CS_PIN          4

// WIZ
#define SPI_WIZ_CS_PIN          3

// ---------- SPI SETUP ----------

// Channels
#define SPI_RAM_CHANNEL        0
#define SPI_SD_CHANNEL         1
#define SPI_RTC_CHANNEL        2
#define SPI_WIZ_CHANNEL        3

// SD Card
#define SPI_SD_CS_MASK        _BV(SPI_SD_CS_PIN)

// SPI RAM
#define SPI_RAM_A0_MASK       _BV(SPI_RAM_A0_PIN)
#define SPI_RAM_A1_MASK       _BV(SPI_RAM_A1_PIN)
#define SPI_RAM_A2_MASK       _BV(SPI_RAM_A2_PIN)
#define SPI_RAM_CS_MASK       _BV(SPI_RAM_CS_PIN)

// RTC
#define SPI_RTC_CS_MASK       _BV(SPI_RTC_CS_PIN)

// WIZ
#define SPI_WIZ_CS_MASK       _BV(SPI_WIZ_CS_PIN)

// All Mask (for init)
#define SPI_ALL_MASK           (SPI_SD_CS_MASK | SPI_RAM_CS_MASK | SPI_RAM_A0_MASK | SPI_RAM_A1_MASK | SPI_RAM_A2_MASK | SPI_RTC_CS_MASK | SPI_WIZ_CS_MASK)

// ---------- RTC SRAM Usage ----------

// WIZ CFG (IP Config)
#define RTC_MEM_OFFSET_WIZ_CFG     0
#define RTC_MEM_SIZE_WIZ_CFG       26

#endif
