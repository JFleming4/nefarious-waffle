all: scheduler

CFLAGS=-D_REENTRANT
LDFLAGS=-lpthread

clean:
	rm -f scheduler
