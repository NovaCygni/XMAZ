all : 
	gcc -Wall --std=c99 -lcrypto -D_BSD_SOURCE -lncurses -o 'XMAZ' *.c
	touch partners
