#ifndef DATA_DEF
#define DATA_DEF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hasher.h"
#include "xor.h"
#include "partner.h"
#include "nonce.h"

#define MESSAGE_SIZE 32
#define NONCE_SIZE 11
#define FLAGS_SIZE 1
#define HASH_SIZE 20
#define DATA_SIZE 64

typedef struct Data Data;
struct Data{
	unsigned char message[MESSAGE_SIZE];
	unsigned char nonce[NONCE_SIZE];
	unsigned char flags;
	unsigned char hash[HASH_SIZE];
	unsigned char data[DATA_SIZE];
};

void newData(Data *data);
void prepareData(Data *data);
void getData(Data *data, unsigned char *buffer);
void encryptData(Data *data, Partner *partner);
void decryptData(Data *data, Partner *partner);
int verifyData(Data *data);
#endif
