#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/ec.h>

#define SECRET_SIZE 66
#define PUBKEY_SIZE 133

typedef struct ECDHE ECDHE;
struct ECDHE{
	EC_KEY *keys;
};

void newECDHE(ECDHE *ecdh){
	if(NULL == (ecdh->keys = EC_KEY_new_by_curve_name(NID_secp521r1))) puts("GENERATE CURVE FAIL");
	if(!EC_KEY_generate_key(ecdh->keys)) puts("GENERATE KEY FAIL");
}

void getPubKey(EC_KEY *keys, unsigned char *buffer){
	u_int8_t pubSize = i2o_ECPublicKey(keys, NULL);
	if(!pubSize) puts("PUBKEY TO DATA ZERO");
	u_int8_t * pubKey = OPENSSL_malloc(pubSize);
	u_int8_t * pubKeyBuff = pubKey;
	if(i2o_ECPublicKey(keys, &pubKeyBuff) != pubSize) puts("PUBKEY TO DATA FAIL");
	for(int i=0;i<pubSize;i++)
		buffer[i]=pubKey[i];
	free(pubKey);
	pubKey=NULL;
}

void computeSecret(EC_KEY *keys, const unsigned char *pubKey, unsigned char *buffer){
	size_t s_len=SECRET_SIZE;
	size_t *secret_len=&s_len;
	unsigned char *secret;
	int field_size;
	long len=PUBKEY_SIZE;
	EC_KEY *peerkey;
	if (NULL == (peerkey=o2i_ECPublicKey(&keys,&pubKey,len))) puts("PUBKEY CONVERSATION FAIL");
	field_size = EC_GROUP_get_degree(EC_KEY_get0_group(keys));
	*secret_len=(field_size+7)/8;
	if(NULL == (secret = OPENSSL_malloc(*secret_len))) puts("SECRET ALLOC FAIL");
	*secret_len = ECDH_compute_key(secret, *secret_len, EC_KEY_get0_public_key(peerkey),keys, NULL);
	if (*secret_len <= 0) puts("KEY COMPUTATION FAIL");
	for(int i=0;i<*secret_len;i++){
		buffer[i]=secret[i];
		secret[i]=0;
	}
	free(secret);	
	secret=NULL;
	EC_KEY_free(peerkey);
}

void ECDHE_free(EC_KEY *keys){
	keys=EC_KEY_new();
	keys=NULL;
}
