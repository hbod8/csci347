# This is a very simple makefile
#

CC = gcc
CFLAGS = -g -Wall
FILES = ush.o expand.o

ush: $(FILES)
	$(CC) $(CFLAGS) -o ush $(FILES)

$(FILES): defn.h

clean:
	rm -f $(FILES) ush