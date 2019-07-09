CC := gcc

SOURCE_DIR  := src
INCLUDE_DIR := includes
OUTPUT_DIR  := build

# Debug Options
# -DDEBUG_MEMORY  ->    Display all memory accesses
# -DDEBUG_LCD     ->    Display information about LCD screen
# -DDISASSEMBLE   ->    Display all commands executed
#DEBUG_FLAGS := -g -DDISASSEMBLE

# Breaks code for some reason
#PROFILE_CODE := -pg

GRAPHICS_LIBRARY := -lSDL2

CFLAGS := -I$(INCLUDE_DIR) $(DEBUG_FLAGS) $(PROFILE_CODE) -Wall
LFLAGS := $(GRAPHICS_LIBRARY) $(PROFILE_CODE)

CFILES := $(wildcard $(SOURCE_DIR)/*.c)
OBJS   := $(patsubst $(SOURCE_DIR)/%.c, $(OUTPUT_DIR)/%.o, $(CFILES))

PROG_NAME := emulator

$(PROG_NAME): $(OUTPUT_DIR) $(OBJS)
	$(CC) $(OUTPUT_DIR)/*.o -o $@ $(LFLAGS)

$(OUTPUT_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)
$(OUTPUT_DIR):
	@mkdir $@

clean:
	rm -rf $(OUTPUT_DIR) $(PROG_NAME)
