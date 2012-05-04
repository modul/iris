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
NAME = project
BUILD = build
BIN = $(BUILD)/bin
OBJ = $(BUILD)/obj
OUTPUT = $(BIN)/$(NAME)

# for release version, override on commandline
TRACE_LEVEL = 5  # DEBUG=5, INFO, WARNING, ERROR, FATAL, NONE=0
OPTFLAGS = -O0 -DDEBUG

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
CFLAGS += $(OPTFLAGS) $(INCLUDES) -D$(CHIP) -DTRACE_LEVEL=$(TRACE_LEVEL)
CFLAGS += -Wall -Wno-format -Wredundant-decls

CFLAGS += -Dprintf=iprintf
CFLAGS += -Dfprintf=fiprintf
CFLAGS += -Dsprintf=siprintf
CFLAGS += -Dscanf=iscanf
CFLAGS += -Dfscanf=fiscanf
CFLAGS += -Dsscanf=siscanf

ASFLAGS += -g -mcpu=cortex-m3 -mthumb -Wall $(OPTFLAGS) $(INCLUDES) -D$(CHIP) -D__ASSEMBLY__

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

#-------------------------------------------------------------------------------
#		Rules
#-------------------------------------------------------------------------------

.PHONY: all clean target program size dist-clean install

all: target

$(BUILD):
	-mkdir $@
	-mkdir $(BIN)
	-mkdir $(OBJ)

target: $(BUILD) $(OUTPUT)

clean:
	-rm $(OBJ)/*.o $(OBJ)/*.lst 

dist-clean:
	-rm -r $(BUILD)

tags: $(C_SRC) $(LIBS)
	ctags --totals -R .

install: program

program: target
	$(OOCD) $(OOCDFLAGS) \
		-c "halt" \
		-c "flash write_bank 0 $(OUTPUT).bin 0" \
		-c "reset run" \
		-c "shutdown"

$(OUTPUT): $(ASM_OBJECTS) $(C_OBJECTS) $(LIBS)
	@echo [LINKING $@]
	@$(CC) $(LDFLAGS) -T"board/flash.ld" \
		   -Wl,-Map,$@.map -o $@.elf  $^ 
	@$(NM) $@.elf >$@.elf.txt
	@$(OBJCOPY) -O binary $@.elf $@.bin
	@$(SIZE) $(filter-out %a, $^) $@.elf
	@$(SIZE) $(filter-out %a, $^) $@.elf > $@-size.txt
	@$(SIZE) -A $(OUTPUT).elf >> $@-size.txt

$(C_OBJECTS): $(OBJ)/%.o: %.c Makefile 
	@echo [COMPILING $<]
	@$(CC) $(CFLAGS) -Dflash -Wa,-ahlms=$(OBJ)/$*.lst -c -o $@ $<  

$(ASM_OBJECTS): $(OBJ)/%.o: %.S Makefile 
	@echo [ASSEMBLING $<]
	@$(CC) $(ASFLAGS) -Dflash -c -o $@ $<

