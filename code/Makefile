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
MEMORIES = flash #sram
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
VPATH += . chip chip/cmsis board

C_SRC += $(wildcard chip/cmsis/*.c)
C_SRC += $(wildcard chip/*.c)
C_SRC += $(wildcard board/*.c)
C_SRC += $(wildcard *.c)

C_OBJECTS  = $(patsubst %.c, %.o, $(notdir $(C_SRC)))

#-------------------------------------------------------------------------------
#		Rules
#-------------------------------------------------------------------------------

.PHONY: all clean $(MEMORIES) runrom runflash program ramboot 

all: $(BIN) $(OBJ) $(MEMORIES)

$(BUILD):
	-mkdir $@

$(BIN) $(OBJ): $(BUILD)
	-mkdir $@

define RULES
C_OBJECTS_$(1) = $(addprefix $(OBJ)/$(1)_, $(C_OBJECTS))
ASM_OBJECTS_$(1) = $(addprefix $(OBJ)/$(1)_, $(ASM_OBJECTS))
TARGET = $(OUTPUT)-$(1).bin

$(1): $$(TARGET)

$$(TARGET): $$(ASM_OBJECTS_$(1)) $$(C_OBJECTS_$(1)) $$(LIBS)
	@echo [LINKING $$@]
	@$(CC) $(LDFLAGS) -T"board/$(1).ld" \
	       	-Wl,-Map,$(OUTPUT)-$(1).map \
	       	-o $(OUTPUT)-$(1).elf $$^ 
	@$(NM) $(OUTPUT)-$(1).elf >$(OUTPUT)-$(1).elf.txt
	@$(OBJCOPY) -O binary $(OUTPUT)-$(1).elf $$@
	@$(SIZE) $$(filter-out %a, $$^) $(OUTPUT)-$(1).elf
	@$(SIZE) $$(filter-out %a, $$^) $(OUTPUT)-$(1).elf > $(OUTPUT)-$(1)-size.txt
	@$(SIZE) -A $(OUTPUT)-$(1).elf >> $(OUTPUT)-$(1)-size.txt

$$(C_OBJECTS_$(1)): $(OBJ)/$(1)_%.o: %.c Makefile 
	@echo [COMPILING $$<]
	@$(CC) $(CFLAGS) -D$(1) -Wa,-ahlms=$(OBJ)/$(1)_$$*.lst -c -o $$@ $$<  

$$(ASM_OBJECTS_$(1)): $(OBJ)/$(1)_%.o: %.S Makefile 
	@echo [ASSEMBLING $$<]
	@$(CC) $(ASFLAGS) -D$(1) -c -o $$@ $$<

endef

$(foreach MEMORY, $(MEMORIES), $(eval $(call RULES,$(MEMORY))))

tags: $(C_SRC) $(LIBS)
	ctags --totals -R . ../libusb ../libchip

clean:
	-rm $(OBJ)/*.o $(OBJ)/*.lst \
			$(BIN)/*.bin $(BIN)/*.elf \
			$(BIN)/*.elf.txt $(BIN)/*.map \
			$(BIN)/*-size.txt

runrom:
	$(OOCD) $(OOCDFLAGS) \
		-c "halt" \
		-c "boot rom" \
		-c "reset run" \
	    -c "shutdown"

runflash:
	$(OOCD) $(OOCDFLAGS) \
		-c "halt" \
		-c "boot flash" \
		-c "reset run" \
		-c "shutdown"

program: flash
	$(OOCD) $(OOCDFLAGS) \
		-c "halt" \
		-c "flash write_bank 0 $(OUTPUT)-flash.bin 0" \
		-c "reset run" \
		-c "shutdown"

ramboot: sram
	$(OOCD) $(OOCDFLAGS) \
		-c "halt" \
		-c "flash write_bank 0 $(OUTPUT)-sram.bin 0" \
		-c "reset run" \
		-c "shutdown"

