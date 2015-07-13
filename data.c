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

void newData(Data *data){
	for(int i=0; i<MESSAGE_SIZE; i++)
		data->message[i]=0;
	for(int i=0; i<NONCE_SIZE; i++)
		data->nonce[i]=0;
	data->flags=0;
	for(int i=0; i<HASH_SIZE; i++)
		data->hash[i]=0;
	for(int i=0; i<DATA_SIZE; i++)
		data->data[i]=0;
}

void prepareData(Data *data){
	Nonce nonce;
	if(data->flags){
		newAckNonce(&nonce);
		for (int i=0; i<MESSAGE_SIZE; i++)
			data->message[i]=nonce.ackNonce[i];
		for (int i=0; i<NONCE_SIZE; i++)
			data->nonce[i]=nonce.ackNonce[i+MESSAGE_SIZE];
	}
	else{
		newNonce(&nonce);
		for (int i=0; i<NONCE_SIZE; i++)
			data->nonce[i]=nonce.nonce[i];
	}
	unsigned char buffer[MESSAGE_SIZE + NONCE_SIZE + FLAGS_SIZE]={0};
	for (int i=0; i< MESSAGE_SIZE; i++)
		buffer[i]=data->message[i];
	for (int i=0; i< NONCE_SIZE; i++)
		buffer[i+MESSAGE_SIZE]=data->nonce[i];
	buffer[MESSAGE_SIZE + NONCE_SIZE]=data->flags;
	hash("ripemd",buffer,data->hash);
	for(int i=0; i<MESSAGE_SIZE; i++)
		data->data[i]=data->message[i];
	for(int i=0; i<NONCE_SIZE; i++)
		data->data[i+MESSAGE_SIZE]=data->nonce[i];
	data->data[MESSAGE_SIZE + NONCE_SIZE]=data->flags;
	for(int i=0; i<HASH_SIZE;i++)
		data->data[i+MESSAGE_SIZE+NONCE_SIZE+FLAGS_SIZE]=data->hash[i];
}

void getData(Data *data, unsigned char *buffer){
	for(int i=0; i<DATA_SIZE;i++)
		buffer[i]=data->data[i];
}

void encryptData(Data *data, Partner *partner){
	xor(data->data,partner->key);
}

void decryptData(Data *data, Partner *partner){
	encryptData(data,partner);
}

int verifyData(Data *data){
	unsigned char realHash[HASH_SIZE]={0};
	unsigned char buffer[MESSAGE_SIZE + NONCE_SIZE + FLAGS_SIZE]={0};
	for (int i=0; i< MESSAGE_SIZE; i++)
		buffer[i]=data->message[i];
	for (int i=0; i< NONCE_SIZE; i++)
		buffer[i+MESSAGE_SIZE]=data->nonce[i];
	buffer[MESSAGE_SIZE + NONCE_SIZE]=data->flags;
	hash("ripemd",buffer,realHash);
	for(int i=0; i<HASH_SIZE; i++)
		if(realHash[i]!=data->hash[i])
			return 0;
	return 1;
}
