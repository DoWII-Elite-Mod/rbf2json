CC = gcc
CFLAGS = -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -fPIC
LIBS = -ljansson

all: rbf2json.c
	$(CC) $(CFLAGS) rbf2json.c -c $(LIBS) 
	ar crus librbf2json.a rbf2json.o
clean: 
	rm rbf2json.o librbf2json.a
