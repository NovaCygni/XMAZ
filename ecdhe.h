#ifndef EC_DEF
#define EC_DEF
#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/ec.h>

#define SECRET_SIZE 66
#define PUBKEY_SIZE 133

typedef struct ECDHE ECDHE;
struct ECDHE{
	EC_KEY *keys;
};

void newECDHE(ECDHE *ecdh);
void getPubKey(EC_KEY *keys, unsigned char *buffer);
void computeSecret(EC_KEY *keys, const unsigned char *pubKey, unsigned char *buffer);
void ECDHE_free(EC_KEY *keys);
#endif
