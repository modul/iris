
SERIE=sam3s
CHIP=$(SERIE)4

LIBNAME=libusb

BIN = lib
OBJ = build

#-------------------------------------------------------------------------------
# Files
#-------------------------------------------------------------------------------

VPATH += source 

INCLUDES = -Iinclude -I../libchip
INCLUDES += -I.. # for trace.h

ifdef DEBUG
PREFIX=$(OBJ)/debug-
LIB=$(BIN)/$(LIBNAME)_$(SERIE)_dbg.a
OPTIMIZATION = -O0 -D DEBUG
TRACE_LEVEL=5
else
PREFIX=$(OBJ)/release-
LIB=$(BIN)/$(LIBNAME)_$(SERIE)_rel.a
OPTIMIZATION = -Os
TRACE_LEVEL=3
endif

C_SRC = $(wildcard source/*.c)
C_OBJ_TEMP = $(patsubst %.c, %.o, $(notdir $(C_SRC)))
C_OBJ_FILTER =
C_OBJ = $(filter-out $(C_OBJ_FILTER), $(C_OBJ_TEMP))
C_OBJECTS = $(addprefix $(PREFIX), $(C_OBJ))

A_SRC = $(wildcard source/*.s)
A_OBJ_TEMP = $(patsubst %.s, %.o, $(notdir $(A_SRC)))
A_OBJ_FILTER =
A_OBJ = $(filter-out $(A_OBJ_FILTER), $(A_OBJ_TEMP))
A_OBJECTS = $(addprefix $(PREFIX), $(A_OBJ))

#-------------------------------------------------------------------------------

CROSS_COMPILE = arm-none-eabi-

# Compilation tools
AR = $(CROSS_COMPILE)ar
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
SIZE = $(CROSS_COMPILE)size
OBJCOPY = $(CROSS_COMPILE)objcopy

# Flags

CFLAGS += -g -mcpu=cortex-m3 -mthumb -mlong-calls 
CFLAGS += -ffunction-sections --param max-inline-insns-single=500 
CFLAGS += $(OPTIMIZATION) $(INCLUDES) -D$(CHIP) -DTRACE_LEVEL=$(TRACE_LEVEL)
CFLAGS += -Dprintf=iprintf
CFLAGS += -Wall -Wno-address -Wno-format

ASFLAGS += -mcpu=cortex-m3 -mthumb -Wall 
ASFLAGS += $(OPTIMIZATION) $(INCLUDES) -D$(CHIP) -D__ASSEMBLY__

#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------------
.PHONY: all clean

all: release

ifndef DEBUG
all: debug
endif

release: $(BIN) $(OBJ) $(LIB)

debug: 
	$(MAKE) DEBUG=1

$(BIN):
	-mkdir $(BIN)
$(OBJ):
	-mkdir $(OBJ)

$(C_OBJECTS): $(PREFIX)%.o: %.c 
	@echo [COMPILING $^]
	@$(CC) -c $(CFLAGS) $< -o $@

$(A_OBJECTS): $(PREFIX)%.o: %.s
	@echo [ASSEMBLING $^]
	@$(AS) -c $(ASFLAGS) $< -o $@

$(LIB): $(C_OBJECTS) $(A_OBJECTS)
	@echo [LINKING]
	@$(AR) -r $@ $^
	@$(SIZE) -t $@ > $(subst .a,-size.txt,$@)

clean:
	-rm  -r $(OBJ) $(BIN)

