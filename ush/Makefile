# This is a very simple makefile
#

CC = gcc
CFLAGS = -g -Wall
FILES = ush.o expand.o builtin.o

ush: $(FILES)
	$(CC) $(CFLAGS) -o ush $(FILES)

$(FILES): defn.h globals.h

clean:
	rm -f $(FILES) ush