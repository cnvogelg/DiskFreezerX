#include "led.h"
#include "floppy-low.h"
#include "timer.h"
#include "util.h"
#include "diskio.h"
#include "sdpin.h"
#include "spi_low.h"

#include "uart.h"
#include "uartutil.h"

#include "cmd_parse.h"

int main(void)
{
    // board setup stuff
    led_init();
    uart_init();
    floppy_init();
    sdpin_init();
    spi_low_cs_init();
    spi_low_mst_init();
    //timer_init();

    // say hello
    uart_send_string((u08 *)"--- dfx-sampler sam7x/SPI ---");
    uart_send_crlf();

    int stay = 1;
    while(stay) {
        uart_send_string((u08 *)"cmd?");
        uart_send_crlf();
        
        led_green(1);
        
        // get next command via SPI
        u08 *cmd;
        u08 len = cmd_uart_get_next(&cmd);

        led_green(0);
        
        uart_send_string((u08 *)"cmd_len: ");
        uart_send_hex_byte_crlf(len);

        if(len>0) {
            u08 result[CMD_MAX_SIZE];
            u08 res_size = CMD_MAX_SIZE;
            
            // parse and execute command
            stay = cmd_parse(len, cmd, &res_size, result);
            
            // report result
            if(res_size > 0) {
                uart_send_string((u08 *)"res_len: ");
                uart_send_hex_byte_crlf(res_size);
                uart_send_data(result, res_size);
                uart_send_crlf();
            }
        }
    }
    
    // say goodbye
    uart_send_string((u08 *)"--- bye ---");
    uart_send_crlf();

    // wait forever... or for a reset
    while(1);
}
