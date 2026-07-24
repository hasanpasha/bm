CC		= gcc
CFLAGS 	= -g \
			-std=c23 \
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
		  -Werror=switch-enum

SRC_DIR	= src
INC_DIR	= include

ifndef BUILD_DIR
BUILD_DIR	= build
endif

CFLAGS += -I${INC_DIR}

TARGET=${BUILD_DIR}/bm

.PHONY: bm run clean

bm: ${TARGET}

${BUILD_DIR}/bm: ${BUILD_DIR}/main.o ${BUILD_DIR}/bm.o
	${CC} ${CFLAGS} -o $@ $^


${BUILD_DIR}/%.o: ${SRC_DIR}/%.c
	${CC} ${CFLAGS} -c -o $@ $<

run: bm
	exec ${TARGET}

clean:
	rm -rf ${BUILD_DIR}/*