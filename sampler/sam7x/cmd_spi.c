#include "cmd_spi.h"
#include "spi.h"
#include "trk_read.h"
#include "uart.h"

#define BUSY_CODE   0x00
#define READY_FLAG  0x80
#define SIZE_MASK   0x3f

#define CMD_NONE    0x00
#define CMD_TX      0x01
#define CMD_RX      0x02
#define CMD_TRK_READ 0x03

static u08 rx_buf[CMD_MAX_SIZE];
static u08 tx_buf[CMD_MAX_SIZE];
static u08 rx_size;
static u08 tx_size;

u08 cmd_uart_get_next(u08 **data)
{
    rx_size = 0;
    while(1) {
        while(!uart_read_ready()) {}
        u08 c;
        uart_read(&c);
        uart_send(c);
        if((c == '\n')||(c == '\r')) {
            break;
        }
        rx_buf[rx_size] = c;
        rx_size++;
    }
    *data = rx_buf;
    return rx_size;
}

u08 cmd_spi_get_next(u08 **data)
{
    // set READY flag   
    spi_write_byte(READY_FLAG | tx_size);

    // now wait for incoming commands
    while(1) {
        u08 cmd = spi_read_byte();
        /* TX from master */
        if(cmd == CMD_TX) {
            u08 size = spi_read_byte();
            for(int i=0;i<size;i++) {
                rx_buf[i] = spi_read_byte();
            }
            
            // clear READY flag
            spi_write_byte(BUSY_CODE);
            
            rx_size = size;
            *data = rx_buf;
            return size;
        } 
        /* RX from master */
        else if(cmd == CMD_RX) {
            u08 size = spi_read_byte();
            for(int i=0;i<size;i++) {
                spi_write_byte(tx_buf[i]);
            }
            
            // reset buffer
            tx_size = 0;
            spi_write_byte(READY_FLAG);
        }
        /* READ_TRK */
        else if(cmd == CMD_TRK_READ) {
            // read track handler
            trk_read_real();
            
            // set to READY
            spi_write_byte(READY_FLAG | tx_size);
        }
    }
}

void cmd_spi_set_result(u08 *data,u08 size)
{
    tx_size = size;
    for(int i=0;i<size;i++) {
        tx_buf[i] = data[i];
    }
}
