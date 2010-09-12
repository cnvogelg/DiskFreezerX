#include "cmdline.h"
#include "uartutil.h"
#include "uart.h"

u08 cmdline[CMDLINE_LEN];

u32 cmdline_getline(void)
{
    uart_send_string((u08 *)"> ");
    
    u32 i = 0;
    while(i<CMDLINE_LEN) {
        // wait for next char
        while(!uart_read_ready());
        
        u08 data;
        uart_read(&data);
        uart_send(data);
        
        if(data == '\r') {
            uart_send('\n');
            break;
        }
        if(data == '\n')
            continue;
            
        cmdline[i++] = data;
    }
    return i;
}
