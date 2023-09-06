CC = gcc
CFLAGS = -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer
LIBS = -ljansson

all: rbf2json.c
	$(CC) $(CFLAGS) rbf2json.c -o bin/rbf2json $(LIBS) 
clean: 
	rm bin/rbf2json
