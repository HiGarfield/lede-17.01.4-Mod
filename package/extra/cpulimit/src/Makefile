CC ?= gcc
CFLAGS ?= -Wall
TARGETS := cpulimit
OBJS := cpulimit.o list.o process_iterator.o process_group.o

CFLAGS += -D_GNU_SOURCE

UNAME ?= $(shell uname)

ifeq ($(UNAME), FreeBSD)
	LDFLAGS += -lkvm
endif

cpulimit: $(OBJS)
	$(CC) -o cpulimit $(OBJS) $(CFLAGS) $(LDFLAGS)

cpulimit.o: cpulimit.c process_group.h list.h
	$(CC) -c cpulimit.c $(CFLAGS)

process_iterator.o: process_iterator.c process_iterator.h process_iterator_linux.c process_iterator_freebsd.c process_iterator_apple.c
	$(CC) -c process_iterator.c $(CFLAGS)

list.o: list.c list.h
	$(CC) -c list.c $(CFLAGS)

process_group.o: process_group.c process_group.h process_iterator.h list.h
	$(CC) -c process_group.c $(CFLAGS)

.PHONY: clean

clean::
	rm -f *~ $(OBJS) $(TARGETS)
