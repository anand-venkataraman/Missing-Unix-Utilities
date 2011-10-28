# Simple Makefile
#
CFLAGS = -O3 -Wall
LIBS = -lcrypto
CC = g++

TARGETS = linecount colsort uniqify

all: $(TARGETS)

%: %.cpp
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

clean:; rm -f $(TARGETS) *.o *~

