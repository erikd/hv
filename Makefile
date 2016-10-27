
CFLAGS = -g -O2 -Wall -Wextra -Werror -Wstrict-prototypes

all: 	hv

clean:
	rm -rf *.o core hv

strip: 	hv
	strip hv

install:
	cp -f hv $(HOME)/Local/bin/

hv: hexdump.o main.o
	gcc	hexdump.o main.o -lncurses -o hv

main.o: main.c hexdump.h
	gcc $(CFLAGS) -c main.c

hexdump.o: hexdump.c hexdump.h
	gcc $(CFLAGS) -c hexdump.c
