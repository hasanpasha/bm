CC=gcc
CFLAGS=-std=c23 -Wall -Wextra -Wswitch-enum -Wmissing-prototypes -pedantic -Iinclude

ifndef BUILD_DIR
BUILD_DIR=build
endif

.PHONY: bme basm debasm test_nan_box examples all clean

all: bme basm debasm test_nan_box examples

bme: ${BUILD_DIR}/bme

${BUILD_DIR}/bme: ${BUILD_DIR}/bme.o ${BUILD_DIR}/bm.o 
	${CC} ${CFLAGS} -o $@ $^

basm: ${BUILD_DIR}/basm

${BUILD_DIR}/basm: ${BUILD_DIR}/basm.o ${BUILD_DIR}/bm.o ${BUILD_DIR}/string_view.o
	${CC} ${CFLAGS} -o $@ $^

debasm: ${BUILD_DIR}/debasm

${BUILD_DIR}/debasm: ${BUILD_DIR}/debasm.o ${BUILD_DIR}/bm.o 
	${CC} ${CFLAGS} -o $@ $^

test_nan_box: ${BUILD_DIR}/test_nan_box

${BUILD_DIR}/test_nan_box: ${BUILD_DIR}/test_nan_box.o ${BUILD_DIR}/nan_box.o 
	${CC} ${CFLAGS} -o $@ $^

${BUILD_DIR}/%.o: src/%.c
	${CC} ${CFLAGS} -c -o $@ $<

examples: basm examples/123.bm examples/fib.bm

examples/%.bm: examples/%.basm
	${BUILD_DIR}/basm $< $@

clean:
	rm -rf ${BUILD_DIR}/*
	rm -rf examples/*.bm