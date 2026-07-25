CC		= gcc
CFLAGS 	= -std=c23 \
          -Wall -Wextra -Wpedantic \
          -Wshadow \
          -Wconversion \
          -Wsign-conversion \
          -Wformat=2 \
          -Wundef \
          -Wstrict-prototypes \
          -Wmissing-prototypes \
          -Werror=enum-conversion \
          -Werror=enum-int-mismatch \
          -Werror=enum-compare \
		  -Werror=switch-enum \
          -Iinclude

ifndef BUILD_DIR
BUILD_DIR	= build
endif

.PHONY: bme basm examples all clean

all: bme basm examples

bme: ${BUILD_DIR}/bme

${BUILD_DIR}/bme: ${BUILD_DIR}/bme.o ${BUILD_DIR}/bm.o 
	${CC} ${CFLAGS} -o $@ $^

basm: ${BUILD_DIR}/basm

${BUILD_DIR}/basm: ${BUILD_DIR}/basm.o ${BUILD_DIR}/bm.o ${BUILD_DIR}/string_view.o
	${CC} ${CFLAGS} -o $@ $^

${BUILD_DIR}/%.o: src/%.c
	${CC} ${CFLAGS} -c -o $@ $<

examples: basm examples/123.bm examples/fib.bm

examples/%.bm: examples/%.basm
	${BUILD_DIR}/basm $< $@

clean:
	rm -rf ${BUILD_DIR}/*
	rm -rf examples/*.bm