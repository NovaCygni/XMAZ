#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

void hash(char *name, unsigned char *value, unsigned char *buffer){
	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	unsigned char md_value[EVP_MAX_MD_SIZE];
	unsigned int md_len;
	size_t length;
	if (strlen(name)==6){
		length=44;
//ripemd160
	}
	else{
		length=64;
//whirlpool
	}

	OpenSSL_add_all_digests();

	md = EVP_get_digestbyname(name);

	mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx, md, NULL);
	EVP_DigestUpdate(mdctx, value, length);
	EVP_DigestFinal_ex(mdctx, md_value, &md_len);
	EVP_MD_CTX_destroy(mdctx);
	for (int i=0; i<md_len; i++)
		buffer[i]=md_value[i];

	EVP_cleanup();
}

