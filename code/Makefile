# -------------------------------------------------------------
#         Remo Giermann (giermann@uni-bonn.de) 
#
# based on a Makefile by 
#         ATMEL Microcontroller Software Support 
#         Copyright (c) 2010, Atmel Corporation
# -------------------------------------------------------------

SERIE = sam3s
CHIP  = $(SERIE)4
BOARD = olimex-sam3-h256
OUTPUT = project
BUILD = build
BIN = $(BUILD)/bin
OBJ = $(BUILD)/obj
OUTPUT := $(BIN)/$(OUTPUT)

# for release version, override on commandline
TRACE_LEVEL = 5  # DEBUG=5, INFO, WARNING, ERROR, FATAL, NONE=0
OPTIMIZATION = -O0 -DDEBUG

#-------------------------------------------------------------------------------
#		Tools
#-------------------------------------------------------------------------------

# make will handle library linking, so no flags for that
USBLIB = -lusb_$(SERIE)_rel
CHIPLIB = -lchip_$(CHIP)_rel
LIBPATHS = libusb/lib libchip/lib
LIBS = $(USBLIB) $(CHIPLIB)

# Tool suffix when cross-compiling
CROSS_COMPILE = arm-none-eabi-

# Compilation tools
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
SIZE = $(CROSS_COMPILE)size
STRIP = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
GDB = $(CROSS_COMPILE)gdb
NM = $(CROSS_COMPILE)nm
OOCD = openocd

INCLUDES  = -Iboard
INCLUDES += -Ilibchip
INCLUDES += -Ilibchip/include
INCLUDES += -Ilibusb
INCLUDES += -Ilibusb/include
INCLUDES += -I.

CFLAGS += -g -mcpu=cortex-m3 -mthumb # -mfix-cortex-m3-ldrd
CFLAGS += -ffunction-sections # might make problems, see manpage
CFLAGS += $(OPTIMIZATION) $(INCLUDES) -D$(CHIP) -DTRACE_LEVEL=$(TRACE_LEVEL)
CFLAGS += -Dprintf=iprintf
CFLAGS += -Dfprintf=fiprintf
CFLAGS += -Dsscanf=siscanf
CFLAGS += -Wall -Wno-format -Wredundant-decls

ASFLAGS += -g -mcpu=cortex-m3 -mthumb -Wall $(OPTIMIZATION) $(INCLUDES) -D$(CHIP) -D__ASSEMBLY__

LDFLAGS += -g -mcpu=cortex-m3 -mthumb 
LDFLAGS += -Wl,--start-group -lgcc -lc -Wl,--end-group
LDFLAGS += -Wl,--entry=ResetException
LDFLAGS += -Wl,--cref -Wl,--check-sections 
LDFLAGS += -Wl,--gc-sections 
LDFLAGS += -Wl,--unresolved-symbols=report-all 
LDFLAGS += -Wl,--warn-common #-Wl,--warn-section-align 
LDFLAGS += -Wl,--warn-unresolved-symbols

OOCDCFG = "openocd.cfg"
OOCDFLAGS = -f $(OOCDCFG) -c init

#-------------------------------------------------------------------------------
#		Files
#-------------------------------------------------------------------------------
vpath %.a $(LIBPATHS)
VPATH += . board

C_SRC += $(wildcard board/*.c)
C_SRC += $(wildcard *.c)

C_OBJECTS = $(addprefix $(OBJ)/, $(patsubst %.c, %.o, $(notdir $(C_SRC))))

TARGET = $(OUTPUT).bin
#-------------------------------------------------------------------------------
#		Rules
#-------------------------------------------------------------------------------

.PHONY: all clean flash program debug

all: $(BIN) $(OBJ) $(TARGET)

$(BUILD):
	-mkdir $@

$(BIN) $(OBJ): $(BUILD)
	-mkdir $@

tags: $(C_SRC) $(LIBS)
	ctags --totals -R .

clean:
	-rm $(OBJ)/*.o $(OBJ)/*.lst \
			$(BIN)/*.bin $(BIN)/*.elf \
			$(BIN)/*.elf.txt $(BIN)/*.map \
			$(BIN)/*-size.txt

program: $(TARGET)
	$(OOCD) $(OOCDFLAGS) \
		-c "halt" \
		-c "flash write_bank 0 $(TARGET) 0" \
		-c "reset run" \
		-c "shutdown"

debug: $(TARGET)
	./debug.sh

$(TARGET): $(ASM_OBJECTS) $(C_OBJECTS) $(LIBS)
	@echo [LINKING $@]
	@$(CC) $(LDFLAGS) -T"board/flash.ld" \
	       	-Wl,-Map,$(OUTPUT).map \
	       	-o $(OUTPUT).elf $^ 
	@$(NM) $(OUTPUT).elf >$(OUTPUT).elf.txt
	@$(OBJCOPY) -O binary $(OUTPUT).elf $@
	@$(SIZE) $(filter-out %a, $^) $(OUTPUT).elf
	@$(SIZE) $(filter-out %a, $^) $(OUTPUT).elf > $(OUTPUT)-size.txt
	@$(SIZE) -A $(OUTPUT).elf >> $(OUTPUT)-size.txt

$(C_OBJECTS): $(OBJ)/%.o: %.c Makefile 
	@echo [COMPILING $<]
	@$(CC) $(CFLAGS) -Dflash -Wa,-ahlms=$(OBJ)/$*.lst -c -o $@ $<  

$(ASM_OBJECTS): $(OBJ)/%.o: %.S Makefile 
	@echo [ASSEMBLING $<]
	@$(CC) $(ASFLAGS) -Dflash -c -o $@ $<

