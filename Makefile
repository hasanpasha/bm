CFLAGS := -std=c11 -Wall -Wextra -Wswitch-enum -Wmissing-prototypes -pedantic -Isrc

ifndef BUILD_DIR
BUILD_DIR := .
endif

TOOLCHAIN := $(patsubst toolchain/%.c,$(BUILD_DIR)/toolchain/%,$(wildcard toolchain/*.c))
EXAMPLES := $(patsubst examples/%.basm,$(BUILD_DIR)/examples/%.bm,$(wildcard examples/*.basm))

.PHONY: all test clean

all: $(TOOLCHAIN) $(EXAMPLES)

$(BUILD_DIR)/toolchain/%: toolchain/%.c | $(BUILD_DIR)/toolchain
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/toolchain:
	mkdir -p $@

$(BUILD_DIR)/examples/%.bm: examples/%.basm $(BUILD_DIR)/toolchain/basm | $(BUILD_DIR)/examples
	$(BUILD_DIR)/toolchain/basm $< $@

$(BUILD_DIR)/examples:
	mkdir -p $@

test: $(TOOLCHAIN) $(EXAMPLES)
	@for example in $(EXAMPLES); do \
		file=$${example##*/}; \
		file=$${file%.bm}; \
		tmp_basm="/tmp/$$file.basm"; \
		tmp_bm="/tmp/$$file.bm"; \
		printf "testing %s: " "$$example"; \
		$(BUILD_DIR)/toolchain/debasm "$$example" > "$$tmp_basm" && \
		$(BUILD_DIR)/toolchain/basm "$$tmp_basm" "$$tmp_bm" && \
		$(BUILD_DIR)/toolchain/bme "$$tmp_bm" > /dev/null && \
		printf "success\n" || { printf "failed\n"; exit 1; }; \
	done

clean:
	rm -f $(TOOLCHAIN) 
	rm -f $(EXAMPLES)