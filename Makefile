CFLAGS := -std=c11 -Wall -Wextra -Wswitch-enum -Wmissing-prototypes -pedantic -Isrc

ifndef BUILD_DIR
BUILD_DIR := .
endif

TOOLCHAIN := $(patsubst toolchain/%.c,$(BUILD_DIR)/toolchain/%,$(wildcard toolchain/*.c))
EXAMPLES := $(patsubst examples/%.basm,$(BUILD_DIR)/examples/%.bm,$(wildcard examples/*.basm))

.PHONY: all clean

all: $(TOOLCHAIN) $(EXAMPLES)

$(BUILD_DIR)/toolchain/%: toolchain/%.c | $(BUILD_DIR)/toolchain
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/toolchain:
	mkdir -p $@

$(BUILD_DIR)/examples/%.bm: examples/%.basm $(BUILD_DIR)/toolchain/basm | $(BUILD_DIR)/examples
	$(BUILD_DIR)/toolchain/basm $< $@

$(BUILD_DIR)/examples:
	mkdir -p $@

clean:
	rm -f $(TOOLCHAIN) 
	rm -f $(EXAMPLES)