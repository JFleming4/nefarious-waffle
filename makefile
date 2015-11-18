all: main

CFLAGS=-D_REENTRANT
LDFLAGS=-lpthread

clean:
	rm -rf main

