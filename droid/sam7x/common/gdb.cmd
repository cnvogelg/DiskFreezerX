target remote localhost:3333
#target remote | /opt/local/bin/openocd --pipe -f interface/openocd-usb.cfg -f target/sam7x256.cfg
#set remote hardware-breakpoint-limit 2
#set remote hardware-watchpoint-limit 2
mon reset init
mon reg pc 0

load /Users/chris/Projects/mcfloppy.git/sam7x/main.elf

