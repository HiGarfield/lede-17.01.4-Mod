CC?=gcc
CFLAGS?=-Wall
TARGETS=cpulimit
LIBS=list.o process_iterator.o process_group.o

CFLAGS += -D_GNU_SOURCE -ffunction-sections -fdata-sections -flto
LDFLAGS += -Wl,--gc-sections,--as-needed,--no-undefined,--no-allow-shlib-undefined -flto

UNAME?=$(shell uname)

ifeq ($(UNAME), FreeBSD)
LIBS+= -lkvm
endif

all::	$(TARGETS) $(LIBS)

cpulimit:	cpulimit.c $(LIBS)
	$(CC) -o cpulimit cpulimit.c $(LIBS) $(CFLAGS) $(LDFLAGS)

process_iterator.o: process_iterator.c process_iterator.h
	$(CC) -c process_iterator.c $(CFLAGS) $(LDFLAGS)

list.o: list.c list.h
	$(CC) -c list.c $(CFLAGS) $(LDFLAGS)

process_group.o: process_group.c process_group.h process_iterator.o list.o
	$(CC) -c process_group.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *~ *.o $(TARGETS)

