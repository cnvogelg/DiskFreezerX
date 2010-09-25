# Tiny Makefile to make a firmware file also bootable from SAM-BA bootloader
#
# I use the Sam_I_Am tool available at:
# http://claymore.engineer.gvsu.edu/~steriana/Software/Sam_I_Am/index.html
#

# config
PROJECT=dfx-sampler
BUILD_DIR=../BUILD

ifdef V
HIDE=
else
HIDE=@
endif

DATA:=data.o
OBJS:=stub.o jump.o $(DATA)

DATA:=$(patsubst %,$(BUILD_DIR)/%,$(DATA))
OBJS:=$(patsubst %,$(BUILD_DIR)/%,$(OBJS))

# fetch t
INPUT=$(BUILD_DIR)/$(PROJECT).hex
WARM=$(BUILD_DIR)/$(PROJECT).warm

NAME=$(BUILD_DIR)/$(PROJECT)-samba
OUT_ELF=$(NAME).elf
OUT_BIN=$(NAME).bin
OUT_HEX=$(NAME).hex
OUT_LSS=$(NAME).lss

# start right after SAM-BA's RAM space
START_ADDR=0x202000

# where to enter the binary from SAM-BA
WARM_START_ADDR=$(shell cat $(WARM))

# where is the Sam_I_Am tool? Assume in path by default.
SAMIAM=Sam_I_Am
SAM_PORT=/dev/ttyUSB0

# ----- rules -----

all: $(OUT_ELF) $(OUT_BIN) $(OUT_HEX) $(OUT_LSS)

# run on target
sam: $(OUT_HEX)
	$(SAMIAM) open $(SAM_PORT) , send "$(OUT_HEX)" , go $(START_ADDR) , disp

clean:
	@echo "  [clean]"
	$(HIDE)rm -f $(OBJS)
	$(HIDE)rm -f $(OUT_ELF) $(OUT_BIN) $(OUT_HEX) $(OUT_LSS)

$(OUT_ELF): $(OBJS)
	@echo "  [link]     $@"
	$(HIDE)arm-elf-ld -T link.map -e $(START_ADDR) --defsym=start_bin=0x$(WARM_START_ADDR) \
	           -o $@ $(OBJS)

%.bin: %.elf
	@echo "  [objcopy]  $@"
	$(HIDE)arm-elf-objcopy --output-target binary $< $@ 

%.hex: %.elf
	@echo "  [objcopy]  $@"
	$(HIDE)arm-elf-objcopy --output-target ihex $< $@

%.lss: %.elf
	@echo "  [objdump]  $@"
	$(HIDE)arm-elf-objdump -h -S -C $< > $@

# convert hex firmware to object file with data
$(DATA): $(INPUT)
	@echo "  [objcopy]  $@ <- $<"
	$(HIDE)arm-elf-objcopy --input-target srec --output-target elf32-littlearm \
			--binary-architecture arm --rename-section .sec1=.data \
			$< $@

$(BUILD_DIR)/%.o: %.S
	@echo "  [asm]      $@"
	$(HIDE)arm-elf-gcc -o $@ -c $<

%.lst: %.o 
	@echo "  [objdump]  $@"
	$(HIDE)arm-elf-objdump -d $< > $@
