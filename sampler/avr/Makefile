#
# Makefile - AVR Makefile
#
# Written by
#  Christian Vogelgsang <chris@vogelgsang.org>
#
# This file is part of dtv2ser.
# See README for copyright notice.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
#  02111-1307  USA.
#

# ----- Config Options -----
 
# install dir of avr libc (here CrossPack location)
AVRLIBC_DIR = /usr/local/CrossPack-AVR/avr/
TOOL_DIR = /usr/local/CrossPack-AVR/bin

ALL_BOARDS= arduino2009
DIST_BOARDS= arduino2009

# select board
BOARD ?= arduino2009
DEFINES ?= USE_DIAGNOSE USE_BOOT USE_JOYSTICK #USE_BLOCKCMD

ifeq "$(BOARD)" "arduino2009"

MCU ?= atmega328
LDR_MCU ?= m328p
F_CPU = 16000000
MAX_ROM_SIZE = 30720
MAX_RAM_SIZE = 2048
UART_BAUD = 57600
#DEFINES += IGNORE_RTS

LDR_PROG = arduino
ifeq "$(MCU)" "atmega168"
LDR_SPEED = 19200
else
LDR_SPEED = 57600
endif

else

$(error "Unsupported board '$(BOARD)'. Only $(ALL_BOARDS) allowed!")

endif

# ----- End of Config -----

# mainfile/project name
PROJECT = dfx-command
VERSION_MAJ = 0
VERSION_MIN = 1
VERSION_EXTRA =

# combine version
VERSION=$(VERSION_MAJ).$(VERSION_MIN)$(VERSION_EXTRA)

# basename of output
BASENAME = $(PROJECT)-$(VERSION)-$(UART_BAUD)-$(BOARD)-$(MCU)
# project
OUTPUT = $(OUTDIR)/$(BASENAME)

# source files
SRC := $(BOARD).c
SRC += util.c uart.c uartutil.c timer.c
SRC += display.c topaz.c console.c main.c
SRC += fat/fatfs.c fat/ff.c fat/mmc.c

# output format
FORMAT = ihex

# build directory
BUILD = BUILD
OBJDIR = $(BUILD)/$(BASENAME)/obj
DEPDIR = $(OBJDIR)
OUTDIR = $(BUILD)

# target files
OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

# compiler switches
CFLAGS = -g -std=gnu99
CFLAGS += -Os
#CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
#CFLAGS += -fno-inline
CFLAGS += -Wall -Werror -Wstrict-prototypes
CFLAGS += -I$(AVRLIBC_DIR)/include
CFLAGS += -mmcu=$(MCU) -I. -Ifat

CFLAGS_LOCAL = -Wa,-adhlns=$(<:%.c=$(OBJDIR)/%.lst)
CFLAGS_LOCAL += -Wp,-M,-MP,-MT,$(OBJDIR)/$(*F).o,-MF,$(DEPDIR)/$(@F:.o=.d)
CFLAGS_LOCAL += -DVERSION_MIN="$(VERSION_MIN)" -DVERSION_MAJ="$(VERSION_MAJ)"
CFLAGS_LOCAL += -DVERSION="\"$(VERSION)\"" -DHAVE_$(BOARD) -DUART_BAUD=$(UART_BAUD)
CFLAGS_LOCAL += -DF_CPU=$(F_CPU)
CFLAGS_LOCAL += $(patsubst %,-D%,$(DEFINES))

# linker switches
LDFLAGS = -Wl,-Map=$(OUTPUT).map,--cref
LDFLAGS = -lm -lc

# Define programs and commands.
SHELL = sh
CC = $(TOOL_DIR)/avr-gcc
OBJCOPY = $(TOOL_DIR)/avr-objcopy
OBJDUMP = $(TOOL_DIR)/avr-objdump
SIZE = $(TOOL_DIR)/avr-size
NM = $(TOOL_DIR)/avr-nm
AVRDUDE = /opt/local/bin/avrdude
REMOVE = rm -f
COPY = cp
RANLIB = avr-ranlib
AR = avr-ar

# define V to see compile output
ifdef V
HIDE=
else
HIDE=@
endif

# ---------- RULES ----------

all: dirs hdr hex lss size

help:
	@echo
	@echo "  supported boards: $(ALL_BOARDS)"
	@echo
	@echo "--- flash bootloader and program fuses ---"
	@echo "  (initial setup of device - only cvm8board!)"
	@echo
	@echo "  > make prog_bootloader"
	@echo "  > make prog_fuse"
	@echo
	@echo "    add ISP_PROG=<programmer>      (e.g. usbasp, see avrdude manual)"
	@echo "    and ISP_PORT='-P <device>'     to configure your ISP"
	@echo	
	@echo "--- flash firmware ---"
	@echo "  (update firmware)"
	@echo
	@echo "  > make prog_cvm8board LDR_PORT=<serial port>"
	@echo "  > make prog_arduino2009 LDR_PORT=<serial port>"
	@echo "         flash the named board attached to given serial port"
	@echo
	@echo "--- build own firmware ---"
	@echo 
	@echo "  > make build BOARD=<boardname>   build firmware for one device"
	@echo "  > make build_all                 build firmware for all devices"
	@echo

dirs:
	@if [ ! -d $(BUILD) ]; then mkdir -p $(BUILD); fi 
	@if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi
	@if [ ! -d $(OBJDIR)/fat ]; then mkdir -p $(OBJDIR)/fat ; fi
	@if [ ! -d $(DEPDIR) ]; then mkdir -p $(DEPDIR); fi
	@if [ ! -d $(OUTDIR) ]; then mkdir -p $(OUTDIR); fi

hdr:
	@echo "--- building BOARD=$(BOARD) F_CPU=$(F_CPU) MCU=$(MCU) ---"
	@echo "             UART_BAUD=$(UART_BAUD) DEFINES=$(DEFINES)"

elf: $(OUTPUT).elf $(OUTPUT).lss $(OUTPUT).sym
	@echo "  --- resulting elf ---"
	$(HIDE)$(SIZE) $(OUTPUT).elf

hex: $(OUTPUT).hex

lss: $(OUTPUT).lss

# size check
size: size_rom size_ram

size_rom: $(OUTPUT).hex
	@SIZE=`$(SIZE) -A --target=$(FORMAT) $(OUTPUT).hex | grep Total | awk '{ print $$2 }'` ; \
	if [ $$SIZE -gt $(MAX_ROM_SIZE) ] ; then \
		echo "  ROM: $$SIZE >  $(MAX_ROM_SIZE) bytes: TOO LARGE" ; exit 1 ; \
	else \
		echo "  ROM: $$SIZE <= $(MAX_ROM_SIZE) bytes: ok" ; \
	fi

size_ram: $(OUTPUT).elf
	@DATA=`$(SIZE) -A $(OUTPUT).elf | fgrep .data | awk '{ print $$2 }'` ; \
	BSS=`$(SIZE) -A $(OUTPUT).elf | fgrep .bss | awk '{ print $$2 }'` ; \
	SIZE=$$(($$DATA + $$BSS)) ; \
	if [ $$SIZE -gt $(MAX_RAM_SIZE) ] ; then \
		echo "  RAM: $$SIZE >  $(MAX_RAM_SIZE) bytes: TOO LARGE" ; exit 1 ; \
	else \
		echo "  RAM: $$SIZE <= $(MAX_RAM_SIZE) bytes: ok" ; \
	fi

# show ram usage
show_ram: $(OUTPUT).elf
	@avr-objdump -t $< | awk '{ print $$4 " " $$5 " " $$6 }' | grep '^\.data' | grep -v '\.data$$' | sort
	@avr-objdump -t $< | awk '{ print $$4 " " $$5 " " $$6 }' | grep '^\.bss' | grep -v '\.bss$$' | sort

# show rom usage
show_rom: $(OUTPUT).elf
	@avr-objdump -t $< | awk '{ print $$4 " " $$5 " " $$6 }' | grep '^\.text' | grep -v '\.text$$' | sort

clean:
	rm -rf $(BUILD)

clean_dist: clean
	rm -rf flash

build_all: clean
	@for a in $(DIST_BOARDS) ; do \
		$(MAKE) build BOARD=$$a ;\
	done
	$(MAKE) build BOARD=arduino2009 MCU=atmega328

dist: build_all
	@rm -rf flash
	@mkdir flash
	@cp $(BUILD)/*.hex flash/
	@rm -rf $(BUILD)
	@ls -la flash/

# ----- Helper Rules -----

# final hex (flash) file from elf
%.hex: %.elf
	@echo "  making hex $@"
	$(HIDE)$(OBJCOPY) -O $(FORMAT) -j .data -j .text $< $@

# finale eeprom file from elf
%.eep: %.elf
	@echo "  making eep $@"
	$(HIDE)$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# extended listing file
%.lss: %.elf
	@echo "  generating listing $@"
	$(HIDE)$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo "  generating symbols $@"
	$(HIDE)$(NM) -n $< > $@

# link
%.elf: $(OBJ)
	@echo "  linking elf $@"
	$(HIDE)$(CC) $(CFLAGS) $(OBJ) --output $@ $(LDFLAGS)

# compile
$(OBJDIR)/%.o : %.c
	@echo "  compiling $<"
	$(HIDE)$(CC) -c $(CFLAGS) $(CFLAGS_LOCAL) $< -o $@ 

$(OBJDIR)/%.o : %.cpp
	@echo "  compiling $<"
	$(HIDE)$(CC) -c $(CFLAGS) $(CFLAGS_LOCAL) $< -o $@ 

# include dependencies
-include $(shell mkdir -p $(DEPDIR) 2>/dev/null) $(wildcard $(DEPDIR)/*.d)

.PRECIOUS: $(OBJ)
.PHONY: all dirs elf hex prog clean avrlib clean.edit hdr


# ----- AVRdude --------------------------------------------------------------

LDR_PORT ?= $(shell ls /dev/cu.usbserial-A7*)

# ISP programmer setup
ISP_PROG ?= usbasp

# commands
AVRDUDE_WRITE_FLASH  = -U flash:w:$(OUTPUT).hex
AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(OUTPUT).eep
AVRDUDE_WRITE_FUSE   = -U lfuse:w:$(LFUSE_$(MCU)):m -U hfuse:w:$(HFUSE_$(MCU)):m

AVRDUDE_ISP_FLAGS = -p $(LDR_MCU) $(ISP_EXTRA) -c $(ISP_PROG)
AVRDUDE_LDR_FLAGS = -p $(LDR_MCU) -P $(LDR_PORT) -b $(LDR_SPEED) -c $(LDR_PROG)

ifdef AVRDUDE_DEBUG
AVRDUDE_ISP_FLAGS += -v -v -v -v
AVRDUDE_LDR_FLAGS += -v -v -v -v
endif

prog: $(OUTPUT).hex size
	@echo "  --- programming flash ---"
	$(AVRDUDE) $(AVRDUDE_LDR_FLAGS) $(AVRDUDE_WRITE_FLASH)

# program a dist hex file with the bootloader
prog_ldr:
	$(AVRDUDE) $(AVRDUDE_LDR_FLAGS) $(AVRDUDE_WRITE_FLASH)

prog_cvm8board:
	$(MAKE) prog_ldr BOARD=cvm8board BUILD?=flash

prog_arduino2009:
	$(MAKE) prog_ldr BOARD=arduino2009 BUILD?=flash

prog_arduino2009_328:
	$(MAKE) prog_ldr BOARD=arduino2009 MCU=atmega328 BUILD?=flash
