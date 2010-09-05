# Makefile for dfx-capture
# 
# cross-compile linux code for Poky Linux running on IGEPv2 board

# config
DEPLOY_DISTRO=poky
DEPLOY_PROJECT=poky-image-sato
DEPLOY_BOARD=igep0020b
DEPLOY_ROOT=/srv/nfs
DEPLOY_PATH=$(DEPLOY_ROOT)/$(DEPLOY_DISTRO)/$(DEPLOY_PROJECT)/$(DEPLOY_BOARD)/test

# prefix for cross-compiler
CROSS_COMPILE=arm-none-linux-gnueabi-

CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc

ifeq "$V" "1"
HIDE=
else
HIDE=@
endif

BUILD_DIR=BUILD

CFLAGS=-g

SRC=dfx-capture.c spi.c spi_bulk.c cmd_spi.c read.c config.c cmd_parse.c
OBJ=$(patsubst %,$(BUILD_DIR)/%,$(SRC:.c=.o))
APP=$(BUILD_DIR)/dfx-capture

all: init $(APP)

init:
	@if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR) ; fi	

$(APP): $(OBJ)
	@echo "  [LD]   $@"
	$(HIDE)$(CC) -o $@ $(OBJ)

$(BUILD_DIR)/%.o: %.c
	@echo "  [CC]   $<"
	$(HIDE)$(CC) -c $(CFLAGS) -o $@ $<

deploy: $(APP)
	@echo "  [DEPLOY] $(APP)"
	@if [ ! -d "$(DEPLOY_PATH)" ]; then \
		echo "DEPLOY_PATH: $(DEPLOY_PATH) not found!"; \
		exit 1; \
	fi
	$(HIDE)cp $(APP) $(DEPLOY_PATH)/

clean:
	@echo "  [CLEAN] $(APP)"
	$(HIDE)rm -rf $(BUILD_DIR)

.PHONY: deploy clean
