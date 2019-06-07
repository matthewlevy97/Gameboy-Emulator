CC := gcc

SOURCE_DIR  := src
INCLUDE_DIR := -Iincludes
OUTPUT_DIR  := build

CFLAGS := $(INCLUDE_DIR) -Wall
LFLAGS := 

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
