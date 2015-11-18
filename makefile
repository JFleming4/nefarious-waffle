all: main scheduler

CFLAGS=-D_REENTRANT
LDFLAGS=-lpthread

clean:
	rm -rf main scheduler
