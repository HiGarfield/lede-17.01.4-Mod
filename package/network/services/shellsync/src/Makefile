CC ?= gcc
override CFLAGS := $(filter-out -std=% -ansi -W%,$(CFLAGS)) \
			-D_GNU_SOURCE -std=c89 \
			-Wall -Wextra -pedantic \
			-Wmissing-prototypes -Wstrict-prototypes \
			-Wold-style-definition
TARGET := shellsync

override LDFLAGS += -lrt -lpthread

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(wildcard *.c *.h)
	$(CC) $(CFLAGS) $(filter %.c, $^) $(LDFLAGS) -o $@

clean:
	rm -f *~ $(TARGET)
