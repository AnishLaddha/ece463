CC = gcc

CFLAGS = -std=gnu99 -g -O3

default: all

all: http_client

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o
	$(CC) -o $@ $^

.PRECIOUS: %.o

.PHONY: default all

clean:
	rm -f http_client; rm make.html; rm reftex.html; rm glpk-4.35.tar.gz;

test:
	./http_client www.gnu.org 80 /software/make/manual/make.html
test2:
	./http_client www.gnu.org 80 /software/auctex/reftex.html

check:
	diff -f make.html make_wget.html

check2:
	diff -f reftex.html reftex_wget.html
#https://ftp.gnu.org/gnu/glpk/glpk-4.35.tar.gz

test_tar:
	./http_client ftp.gnu.org 80 /gnu/glpk/glpk-4.35.tar.gz

check_tar:
	diff -f glpk-4.35.tar.gz glpk-4.35_wget.tar.gz