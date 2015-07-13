#include <stdio.h>
#include <stdlib.h>
#include <openssl/rand.h>
#define MESSAGE_SIZE 32
#define DEFAULT_LENGTH 11
#define ACK_LENGTH MESSAGE_SIZE+DEFAULT_LENGTH

typedef struct Nonce Nonce;
struct Nonce{
	unsigned char nonce[DEFAULT_LENGTH];
	unsigned char ackNonce[ACK_LENGTH];
};

void newNonce(Nonce *nonce){
	RAND_bytes(nonce->nonce,DEFAULT_LENGTH);
}
void newAckNonce(Nonce *nonce){
	RAND_bytes(nonce->ackNonce,ACK_LENGTH);
}

