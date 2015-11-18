all : 
	touch partners
	sed -i 's?\"partners\"?\"'`pwd`'\/partners\"?' partner.*
	sed -i 's?\"ASCII?\"'`pwd`'\/ASCII?' chat.c
	gcc -Wall --std=c99 -lcrypto -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200112L -lncurses -o 'XMAZ' *.c
