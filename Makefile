.PHONY: all, clean

# Disable implicit rules
.SUFFIXES:

CC=gcc
CFLAGS=-Wall -g
VPATH=src/

# Note: -lnsl does not seem to work on Mac OS but will
# probably be necessary on Solaris for linking network-related functions 
# LIBS += -lsocket -lnsl -lrt
LIBS+=-lpthread

INCLUDE = readcmd.h list_jobs.h csapp.h
OBJS = readcmd.o list_jobs.o csapp.o
INCLDIR = -I.

#rajouter d'autres cibles ou taper directement make <nom d'executable>
PROGS = shell

all: $(PROGS)

%.o: %.c $(INCLUDE)
	$(CC) $(CFLAGS) $(INCLDIR) -c -o $@ $<

%: %.o $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

clean:
	rm -f shell *.o

