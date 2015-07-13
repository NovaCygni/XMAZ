#include <stdio.h>
#include <stdlib.h>
#define DATA_SIZE 64

void xor(unsigned char *string1, unsigned char *string2){
	for( int i=0; i<DATA_SIZE ; i++)
		string1[i] ^= string2[i];
}
