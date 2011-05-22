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
#define BUTTON1_PIN             19
#define BUTTON2_PIN             20

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
#define MOTOR_ENABLE_PIN        23
#define DRIVE_SELECT_PIN        10

// IN
#define READ_DATA_PIN           26
#define TRACK_ZERO_PIN          9
#define INDEX_PIN               30

// SPI (reserved)
#define SPI_MISO_PIN            12
#define SPI_MOSI_PIN            13
#define SPI_CLK_PIN             14

// SD Card
#define SD_SOCKET_WP_PIN        25
#define SD_SOCKET_INS_PIN       15
#define SPI_CS0_PIN             11

// SPI RAM
#define SPI_MULTI_A0_PIN        5
#define SPI_MULTI_A1_PIN        6
#define SPI_MULTI_A2_PIN        7
#define SPI_MULTI_CS_PIN        8

#endif
