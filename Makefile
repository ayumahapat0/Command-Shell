CC = gcc
CFLAGS = -Wall -Werror -pedantic -std=gnu18
LOGIN = amahapatra
SUBMITPATH = ~cs537-1/handin/amahapatra/P3

all: wsh

wsh: wsh.c wsh.h
	$(CC) $(FLAGS) -o $@ $^

run: wsh
	./wsh

pack:
	tar -czvf $(LOGIN).tar.gz wsh.h wsh.c Makefile README.md slipdays.txt

submit: pack
	cp $(LOGIN).tar.gz $(SUBMITPATH)