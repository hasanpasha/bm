CFLAGS:=-std=c23 -Wall -Wextra -Wswitch-enum -Wmissing-prototypes -pedantic -Iinclude

ifndef BUILD_DIR
BUILD_DIR:=build
endif

TOOLCHAIN:=$(patsubst src/%.c,$(BUILD_DIR)/toolchain/%,$(wildcard src/*.c))
EXAMPLES:=$(patsubst examples/%.basm,$(BUILD_DIR)/examples/%.bm,$(wildcard examples/*.basm))

.PHONY: all toolchain examples clean

all: toolchain examples

toolchain: $(TOOLCHAIN)

$(BUILD_DIR)/toolchain/%: src/%.c | $(BUILD_DIR)/toolchain
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/toolchain:
	mkdir -p $@

examples: $(BUILD_DIR)/toolchain/basm $(EXAMPLES)

$(BUILD_DIR)/examples/%.bm: examples/%.basm | $(BUILD_DIR)/examples
	$(BUILD_DIR)/toolchain/basm $< $@

$(BUILD_DIR)/examples:
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)