# Tiny Makefile to generate the stub code for sam-ba-wrap.py

PROJECT=dfx-sampler
DATA=data.o
OBJS=stub.o jump.o $(DATA)
INPUT=../BUILD/$(PROJECT).hex
WARM=../BUILD/$(PROJECT).warm
OUT_ELF=$(PROJECT)-samba.elf
OUT_BIN=$(PROJECT)-samba.bin
OUT_HEX=$(PROJECT)-samba.hex
OUT_LSS=$(PROJECT)-samba.lss

# start right after SAM-BA's RAM space
START_ADDR=0x202000

# where to enter the binary from SAM-BA
WARM_START_ADDR=$(shell cat $(WARM))

all: $(OUT_ELF) $(OUT_BIN) $(OUT_HEX) $(OUT_LSS)

clean:
	rm -f $(OBJS)
	rm -f $(OUT_ELF) $(OUT_BIN) $(OUT_HEX) $(OUT_LSS)

$(OUT_ELF): $(OBJS)
	arm-elf-ld -T link.map -e $(START_ADDR) --defsym=start_bin=0x$(WARM_START_ADDR) \
	           -o $@ $(OBJS)

%.bin: %.elf
	arm-elf-objcopy --output-target binary $< $@ 

%.hex: %.elf
	arm-elf-objcopy --output-target ihex $< $@

%.lss: %.elf
	arm-elf-objdump -h -S -C $< > $@

# convert hex firmware to object file with data
$(DATA): $(INPUT)
	arm-elf-objcopy --input-target srec --output-target elf32-littlearm \
			--binary-architecture arm --rename-section .sec1=.data \
			$< $@

%.o: %.S
	arm-elf-gcc -c $<

%.lst: %.o 
	arm-elf-objdump -d $< > $@
