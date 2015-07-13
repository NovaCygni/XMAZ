#ifndef DEF_NONCE
#define DEF_NONCE
#define MESSAGE_SIZE 32
#define DEFAULT_LENGTH 11
#define ACK_LENGTH MESSAGE_SIZE+DEFAULT_LENGTH

typedef struct Nonce Nonce;
struct Nonce{
        unsigned char nonce[DEFAULT_LENGTH];
	unsigned char ackNonce[ACK_LENGTH];
};

void newNonce(Nonce *nonce);
void newAckNonce(Nonce *nonce);

#endif
