ifndef CC
CC=gcc
endif

OBJECTS = example.o cluster.o
SRCDIR = ../src

example.exe: $(OBJECTS)
	$(CC) -Wall $(OBJECTS) -o example.exe

example.o: example.c $(SRCDIR)/cluster.h
	$(CC) -c -Wall -I$(SRCDIR) example.c

cluster.o: $(SRCDIR)/cluster.c $(SRCDIR)/cluster.h
	$(CC) -c -Wall $(SRCDIR)/cluster.c

clean:
	rm -f example.exe example.o cluster.o
