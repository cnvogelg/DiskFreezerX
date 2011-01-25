#ifndef mmc_h_
#define mmc_h_


//Pins
#if (defined(__AVR_ATmega1280__) || \
     defined(__AVR_ATmega1281__) || \
     defined(__AVR_ATmega2560__) || \
     defined(__AVR_ATmega2561__))      //--- Arduino Mega ---

# define SD_CS_PORT          PORTG
# define SD_CS_DDR           DDRG
# define SD_CS_PIN           (5) //4 or 10
# define SD_DESELECT()       PORTG |=  (1<<5) //digitalWrite(SD_CS_PIN, HIGH)
# define SD_SELECT()         PORTG &= ~(1<<5) //digitalWrite(SD_CS_PIN, LOW)

# define SD_MOSI_PORT	     PORTB
# define SD_MOSI_DDR		 DDRB
# define SD_MOSI_PIN         (2)
# define SD_MOSI_HIGH()      PORTB |=  (1<<2) //digitalWrite(SD_MOSI_PIN, HIGH)
# define SD_MOSI_LOW()       PORTB &= ~(1<<2) //digitalWrite(SD_MOSI_PIN, LOW)

# define SD_MOSI_PORT		 PORTB
# define SD_MOSI_DDR		 DDRB
# define SD_MISO_PIN         (3)
# define SD_MISO_READ()      (PINB & (1<<3))

# define SD_CLK_PORT		 PORTB
# define SD_CLK_DDR			 DDRB
# define SD_CLK_PIN          (1)
# define SD_CLK_HIGH()       PORTB |=  (1<<1) //digitalWrite(SD_CLK_PIN, HIGH)
# define SD_CLK_LOW()        PORTB &= ~(1<<1) //digitalWrite(SD_CLK_PIN, LOW)

#else                                  //--- Arduino Uno ---

# define SD_CS_PORT          PORTD
# define SD_CS_DDR           DDRD
# define SD_CS_PIN           (4) //4 or 10
# define SD_DESELECT()       PORTD |=  (1<<4) //digitalWrite(SD_CS_PIN, HIGH)
# define SD_SELECT()         PORTD &= ~(1<<4) //digitalWrite(SD_CS_PIN, LOW)

# define SD_MOSI_PORT        PORTB
# define SD_MOSI_DDR         DDRB
# define SD_MOSI_PIN         (3)
# define SD_MOSI_HIGH()      PORTB |=  (1<<3) //digitalWrite(SD_MOSI_PIN, HIGH)
# define SD_MOSI_LOW()       PORTB &= ~(1<<3) //digitalWrite(SD_MOSI_PIN, LOW)

# define SD_MISO_PORT        PORTB
# define SD_MISO_DDR         DDRB
# define SD_MISO_PIN         (4)
# define SD_MISO_READ()      (PINB & (1<<4))

# define SD_CLK_PORT         PORTB
# define SD_CLK_DDR          DDRB
# define SD_CLK_PIN          (5)
# define SD_CLK_HIGH()       PORTB |=  (1<<5) //digitalWrite(SD_CLK_PIN, HIGH)
# define SD_CLK_LOW()        PORTB &= ~(1<<5) //digitalWrite(SD_CLK_PIN, LOW)

#endif


//MMC/SD command
#define CMD0                 (0x40+ 0) //GO_IDLE_STATE
#define CMD1                 (0x40+ 1) //SEND_OP_COND
#define	ACMD41               (0xC0+41) //SEND_OP_COND (SDC)
#define CMD8                 (0x40+ 8) //SEND_IF_COND
#define CMD9                 (0x40+ 9) //SEND_CSD
#define CMD10                (0x40+10) //SEND_CID
#define CMD12                (0x40+12) //STOP_TRANSMISSION
#define ACMD13               (0xC0+13) //SD_STATUS (SDC)
#define CMD16                (0x40+16) //SET_BLOCKLEN
#define CMD17                (0x40+17) //READ_SINGLE_BLOCK
#define CMD18                (0x40+18) //READ_MULTIPLE_BLOCK
#define CMD23                (0x40+23) //ET_BLOCK_COUNT
#define	ACMD23               (0xC0+23) //SET_WR_BLK_ERASE_COUNT (SDC)
#define CMD24                (0x40+24) //WRITE_BLOCK
#define CMD25                (0x40+25) //WRITE_MULTIPLE_BLOCK
#define CMD41                (0x40+41) //SEND_OP_COND (ACMD)
#define CMD55                (0x40+55) //APP_CMD
#define CMD58                (0x40+58) //READ_OCR

//Card type flags
#define CT_MMC               (0x01)
#define CT_SD1               (0x02)
#define CT_SD2               (0x04)
#define CT_SDC               (CT_SD1|CT_SD2)
#define CT_BLOCK             (0x08)

//Prototypes
//DSTATUS                      disk_initialize(UCHAR drv);
//DSTATUS                      disk_status (UCHAR drv);
//DRESULT                      disk_read (UCHAR drv, UCHAR *buff, DWORD sector, UCHAR count);
//DRESULT                      disk_write(UCHAR drv, const UCHAR *buff, DWORD sector, UCHAR count);
//DRESULT                      disk_ioctl(UCHAR drv, UCHAR ctrl, void *buff);
void                         disk_timerproc(void);
DWORD                        get_fattime(void);


#endif //mmc_h_
