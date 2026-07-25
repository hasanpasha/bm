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

.PHONY: bmi ebasm examples all clean

all: bmi ebasm examples

bmi: ${BUILD_DIR}/bmi

${BUILD_DIR}/bmi: ${BUILD_DIR}/bmi.o ${BUILD_DIR}/bm.o 
	${CC} ${CFLAGS} -o $@ $^

ebasm: ${BUILD_DIR}/ebasm

${BUILD_DIR}/ebasm: ${BUILD_DIR}/ebasm.o ${BUILD_DIR}/bm.o ${BUILD_DIR}/string_view.o
	${CC} ${CFLAGS} -o $@ $^

${BUILD_DIR}/%.o: src/%.c
	${CC} ${CFLAGS} -c -o $@ $<

examples: ebasm examples/123.bm examples/fib.bm

examples/%.bm: examples/%.ebasm
	${BUILD_DIR}/ebasm $< $@

clean:
	rm -rf ${BUILD_DIR}/*
	rm -rf examples/*.bm