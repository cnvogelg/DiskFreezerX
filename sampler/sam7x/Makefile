# Hey Emacs, this is a -*- makefile -*-
#
# WinARM template makefile 
# by Martin Thomas, Kaiserslautern, Germany 
# <eversmith@heizung-thomas.de>
#
# based on the WinAVR makefile written by Eric B. Weddington, Jörg Wunsch, et al.
# Released to the Public Domain
# Please read the make user manual!
#
#
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make program = Download the hex file to the device
#
# (TODO: make filename.s = Just compile filename.c into the assembler code only)
#
# To rebuild project do "make clean" then "make all".
#
# Changelog:
# - 17. Feb. 2005  - added thumb-interwork support (mth)
# - 28. Apr. 2005  - added C++ support (mth)
# - 29. Arp. 2005  - changed handling for lst-Filename (mth)
# -  1. Nov. 2005  - exception-vector placement options (mth)
# - 15. Nov. 2005  - added library-search-path (EXTRA_LIB...) (mth)
# -  2. Dec. 2005  - fixed ihex and binary file extensions (mth)
# - 22. Feb. 2006  - added AT91LIBNOWARN setting (mth)
# - 19. Apr. 2006  - option FLASH_TOOL (default lpc21isp); variable IMGEXT (mth)
# - 23. Jun. 2006  - option USE_THUMB_MODE -> THUMB/THUMB_IW
# -  3. Aug. 2006  - added -ffunction-sections -fdata-sections to CFLAGS
#                    and --gc-sections to LDFLAGS. Only availble for gcc 4 (mth)
# -  4. Aug. 2006  - pass SUBMDL-define to frontend (mth)

#FLASH_TOOL = UVISION
FLASH_TOOL = OPENOCD

# MCU name and submodel
MCU      = arm7tdmi
#SUBMDL   = AT91SAM7S64
SUBMDL   = AT91SAM7S256

#USE_THUMB_MODE = YES
USE_THUMB_MODE = NO

## Create ROM-Image (final)
#RUN_MODE=ROM_RUN
## Create RAM-Image (debugging) 
##( not used: example does not fit in AT91SAM7S64 RAM )
RUN_MODE=RAM_RUN

## Exception-Vector placement only supported for "ROM_RUN"
## (placement settings ignored when using "RAM_RUN")
## - Exception vectors in ROM:
#VECTOR_LOCATION=VECTORS_IN_ROM
## - Exception vectors in RAM:
VECTOR_LOCATION=VECTORS_IN_RAM

# Target file name (without extension).
TARGET = dfx-sampler

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files
SRC += spi_low.c cmd_parse.c trk_read.c pit.c spiram.c disk.c button.c
SRC += led.c delay.c floppy-low.c uart.c uartutil.c util.c cmdline.c timer.c 
SRC += ff.c diskio_spi.c file.c ccsbcs.c sdpin.c spi.c track.c memory.c floppy.c
SRC += rtc.c wiz_low.c wiz.c net.c
SRC += $(TARGET).c

# List C source files here which must be compiled in ARM-Mode.
# use file-extension c for "c-only"-files
SRCARM = common/Cstartup_SAM7.c
#SRCARM += timer_irq.c 
#SRCARM += irqutils.c

# List C++ source files here.
# use file-extension cpp for C++-files (use extension .cpp)
CPPSRC = 

# List C++ source files here which must be compiled in ARM-Mode.
# use file-extension cpp for C++-files (use extension .cpp)
CPPSRCARM = 

# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC = 

# List Assembler source files here which must be assembled in ARM-Mode..
ASRCARM  = common/Cstartup.S
#ASRCARM += common/swi_handler.S

## Output format. (can be ihex or binary)
## (binary i.e. for openocd and SAM-BA, hex i.e. for lpc21isp and uVision)
FORMAT = ihex
#FORMAT = binary

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
#OPT = s
OPT = 2

# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
#DEBUG = stabs
DEBUG = dwarf-2

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS = ./common

# List any extra directories to look for library files here.
#     Each directory must be seperated by a space.
#EXTRA_LIBDIRS = ../arm7_efsl_0_2_4
EXTRA_LIBDIRS = ./common

## Using the Atmel AT91_lib produces warning with
## the default warning-levels. 
## yes - disable these warnings; no - keep default settings
AT91LIBNOWARN = yes
#AT91LIBNOWARN = no

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Place -D or -U options for C here
CDEFS =  -D$(RUN_MODE)

# Place -I options here
CINCS =

# Place -D or -U options for ASM here
ADEFS =  -D$(RUN_MODE)

ifdef VECTOR_LOCATION
CDEFS += -D$(VECTOR_LOCATION)
ADEFS += -D$(VECTOR_LOCATION)
endif

CDEFS += -D__WinARM__ -D__WINARMSUBMDL_$(SUBMDL)__
ADEFS += -D__WinARM__ -D__WINARMSUBMDL_$(SUBMDL)__

ifdef V
HIDE := 
else
HIDE := @
endif

# Compiler flags.

ifeq ($(USE_THUMB_MODE),YES)
THUMB    = -mthumb
THUMB_IW = -mthumb-interwork
else 
THUMB    = 
THUMB_IW = 
endif

#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS = -g$(DEBUG)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT)
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wall -Wcast-align -Wimplicit -Werror
CFLAGS += -Wpointer-arith -Wswitch
CFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
CFLAGS += -Wa,-adhlns=$(patsubst %,$(BUILD_DIR)/%,$(subst $(suffix $<),.lst,$<)) 
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

# flags only for C
CONLYFLAGS += -Wnested-externs 
CONLYFLAGS += $(CSTANDARD)

ifneq ($(AT91LIBNOWARN),yes)
#AT91-lib warnings with:
CFLAGS += -Wcast-qual
CONLYFLAGS += -Wmissing-prototypes 
CONLYFLAGS += -Wstrict-prototypes
CONLYFLAGS += -Wmissing-declarations
endif

# flags only for C++ (arm-elf-g++)
CPPFLAGS = -fno-rtti -fno-exceptions

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
#  -g$(DEBUG): have the assembler create line number information
ASFLAGS = $(ADEFS) -Wa,-adhlns=$(patsubst %,$(BUILD_DIR)/%,$(<:.S=.lst)),--g$(DEBUG)


#Additional libraries.

# Extra libraries
#    Each library-name must be seperated by a space.
#    To add libxyz.a, libabc.a and libefsl.a: 
#    EXTRA_LIBS = xyz abc efsl
#EXTRA_LIBS = efsl
EXTRA_LIBS =

#Support for newlibc-lpc (file: libnewlibc-lpc.a)
#NEWLIBLPC = -lnewlib-lpc

MATH_LIB = -lm

# CPLUSPLUS_LIB = -lstdc++

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -nostartfiles -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref,--gc-sections
LDFLAGS += -lc
LDFLAGS += $(NEWLIBLPC) $(MATH_LIB)
LDFLAGS += -lc -lgcc 
LDFLAGS += $(CPLUSPLUS_LIB)
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))

# Set Linker-Script Depending On Selected Memory and Controller
ifeq ($(RUN_MODE),RAM_RUN)
LDFLAGS +=-T./common/$(SUBMDL)-RAM.ld
else 
LDFLAGS +=-T./common/$(SUBMDL)-ROM.ld
endif

# Define programs and commands.
SHELL = sh
CC = arm-elf-gcc-4.6
CPP = arm-elf-g++
OBJCOPY = arm-elf-objcopy
OBJDUMP = arm-elf-objdump
SIZE = arm-elf-size
NM = arm-elf-nm
REMOVE = rm -f
COPY = cp

BUILD_DIR=BUILD

# Define all object files.
COBJ      = $(patsubst %,$(BUILD_DIR)/%,$(SRC:.c=.o)) 
AOBJ      = $(patsubst %,$(BUILD_DIR)/%,$(ASRC:.S=.o))
COBJARM   = $(patsubst %,$(BUILD_DIR)/%,$(SRCARM:.c=.o))
AOBJARM   = $(patsubst %,$(BUILD_DIR)/%,$(ASRCARM:.S=.o))
CPPOBJ    = $(patsubst %,$(BUILD_DIR)/%,$(CPPSRC:.cpp=.o)) 
CPPOBJARM = $(patsubst %,$(BUILD_DIR)/%,$(CPPSRCARM:.cpp=.o))

# Define all listing files.
LST := $(ASRC:.S=.lst) $(ASRCARM:.S=.lst) $(SRC:.c=.lst) $(SRCARM:.c=.lst)
LST += $(CPPSRC:.cpp=.lst) $(CPPSRCARM:.cpp=.lst)
LST := $(patsubst %,$(BUILD_DIR)/%,$(LST))

# Compiler flags to generate dependency files.
GENDEPFLAGS = -MD -MP -MF $(BUILD_DIR)/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS  = -mcpu=$(MCU) $(THUMB_IW) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. -x assembler-with-cpp $(ASFLAGS)

TARGET_ELF=$(BUILD_DIR)/$(TARGET).elf
TARGET_LSS=$(BUILD_DIR)/$(TARGET).lss
TARGET_SYM=$(BUILD_DIR)/$(TARGET).sym
TARGET_HEX=$(BUILD_DIR)/$(TARGET).hex
TARGET_BIN=$(BUILD_DIR)/$(TARGET).bin
TARGET_WARM=$(BUILD_DIR)/$(TARGET).warm

# Default target.
all: make_build build_all calcsize

make_build:
	@if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR) ; fi
	@if [ ! -d $(BUILD_DIR)/common ]; then mkdir $(BUILD_DIR)/common ; fi

ifeq ($(FORMAT),ihex)
build_all: elf hex lss sym warm
hex: $(TARGET_HEX)
IMGEXT=hex
else 
ifeq ($(FORMAT),binary)
build_all: elf bin lss sym
bin: $(TARGET_BIN)
IMGEXT=bin
else 
$(error "Unknown Format: $(FORMAT)")
endif
endif

elf: $(TARGET_ELF)
lss: $(TARGET_LSS) 
sym: $(TARGET_SYM)
warm: $(TARGET_WARM)

# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET_HEX)
ELFSIZE = $(SIZE) -B $(TARGET_ELF)
calcsize:
	@if [ -f $(TARGET_ELF) ]; then \
		$(ELFSIZE) ; \
	fi

# Display compiler version information.
gccversion : 
	@$(CC) --version

run: $(TARGET).$(IMGEXT)
	$(HIDE)oocd -f common/run_ram.ocd

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
	@echo "  [objcopy]  $@"
	$(HIDE)$(OBJCOPY) -O $(FORMAT) $< $@

# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@echo "  [objcopy]  $@"
	$(HIDE)$(OBJCOPY) -O $(FORMAT) $< $@


# Create extended listing file from ELF output file.
# testing: option -C
%.lss: %.elf
	@echo "  [objcopy]  $@"
	$(HIDE)$(OBJDUMP) -h -S -C $< > $@


# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo "  [nm]       $@"
	$(HIDE)$(NM) -n $< > $@

# Extract warm start address
%.warm: %.elf
	$(HIDE)$(OBJDUMP) -t $< | grep warm_start | awk '{ print $$1 }' > $@
	@echo "  [objcopy]  $@ (extract warm start address)"
	@cat $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET_ELF)
.PRECIOUS : $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)
%.elf:  $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)
	@echo "  [link]     $@"
	$(HIDE)$(CC) $(THUMB) $(ALL_CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) --output $@ $(LDFLAGS)

# Compile: create object files from C source files. ARM/Thumb
$(COBJ) : $(BUILD_DIR)/%.o : %.c
	@echo "  [cc,thumb] $<"
	$(HIDE)$(CC) -c $(THUMB) $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@ 

# Compile: create object files from C source files. ARM-only
$(COBJARM) : $(BUILD_DIR)/%.o : %.c
	@echo "  [cc,arm]   $<"
	$(HIDE)$(CC) -c $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@ 

# Compile: create object files from C++ source files. ARM/Thumb
$(CPPOBJ) : $(BUILD_DIR)/%.o : %.cpp
	@echo "  [c++thumb] $<"
	$(HIDE)$(CPP) -c $(THUMB) $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@ 

# Compile: create object files from C++ source files. ARM-only
$(CPPOBJARM) : $(BUILD_DIR)/%.o : %.cpp
	@echo "  [c++arn]   $<"
	$(HIDE)$(CPP) -c $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@ 

# Assemble: create object files from assembler source files. ARM/Thumb
$(AOBJ) : $(BUILD_DIR)/%.o : %.S
	@echo "  [as,thumb] $<"
	$(HIDE)$(CC) -c $(THUMB) $(ALL_ASFLAGS) $< -o $@

# Assemble: create object files from assembler source files. ARM-only
$(AOBJARM) : $(BUILD_DIR)/%.o : %.S
	@echo "  [as,arm]   $<"
	$(HIDE)$(CC) -c $(ALL_ASFLAGS) $< -o $@


# Target: clean project.
clean: clean_list

clean_list :
	@echo "  [clean]"
	@rm -rf $(BUILD_DIR)

# Include the dependency files.
-include $(wildcard $(BUILD_DIR)/*.d)


# Listing of phony targets.
.PHONY : all calcsize gccversion \
build elf hex bin lss sym clean clean_list program

