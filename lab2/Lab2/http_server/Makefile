CC = gcc

CFLAGS = -std=gnu99 -g -O3

default: all

all: http_server db_server udp_test

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o
	$(CC) -o $@ $^

.PRECIOUS: %.o

.PHONY: default all

clean:
	rm -f http_server; rm -f db_server; rm -r http_server.dSYM; rm -r db_server.dSYM; rm -f upd_test;
