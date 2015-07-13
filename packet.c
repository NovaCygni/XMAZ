#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "partner.h"

#define MESSAGE_SIZE 32
#define NONCE_SIZE 11
#define FLAGS_SIZE 1
#define HASH_SIZE 20
#define DATA_SIZE 64
#define RECODE 2
#define PREV_CODE 3
#define BAD_CODE 4

void formPacket(char *message, unsigned char *datagram, Partner *partner){
	Data data;
	newData(&data);
	data.flags=partner->flags;
	if(!partner->flags){
		for (int i=0; i<MESSAGE_SIZE; i++)
			data.message[i]=message[i];
	}
	prepareData(&data);
/*
	printf("NONCE : ");
	for (int i=0; i<NONCE_SIZE; i++)
		printf("%02x",data.nonce[i]);
	printf("\n");
*/
	unsigned char buffer[MESSAGE_SIZE + NONCE_SIZE + FLAGS_SIZE];
	for(int i=0;i<sizeof(buffer);i++)
		buffer[i]=0;
	for (int i=0; i< MESSAGE_SIZE; i++)
		buffer[i]=data.message[i];
	for (int i=0; i< NONCE_SIZE; i++)
		buffer[i+MESSAGE_SIZE]=data.nonce[i];
	buffer[MESSAGE_SIZE + NONCE_SIZE]=data.flags;
/*
	printf("MESSAGE + NONCE + FLAGS : ");
	for (int i=0; i< MESSAGE_SIZE + NONCE_SIZE + FLAGS_SIZE; i++)
		printf("%02x",buffer[i]);
	printf("\n");
	printf("HASH ( MESSAGE + NONCE + FLAGS ): ");
	for(int i=0; i<HASH_SIZE; i++)
		printf("%02x",data.hash[i]);
	printf("\n");
	printf("DATA : ");
	for(int i=0; i<DATA_SIZE; i++)
                printf("%02x",data.data[i]);
        printf("\n");
*/
	encryptData(&data,partner);
/*
	printf("KEY : ");
        for (int i=0; i<DATA_SIZE;i++)
                printf("%02x",partner->key[i]);
        printf("\n");
	printf("XORED DATA : ");
	for(int i=0; i<DATA_SIZE; i++)
                printf("%02x",data.data[i]);
        printf("\n");
*/
	if(partner->flags){
		nextKey(partner);
		update(partner);
/*
	printf("NEXT KEY : ");
        for (int i=0; i<DATA_SIZE;i++)
                printf("%02x",partner->key[i]);
        printf("\n");
*/
	}
	for(int i=0; i<DATA_SIZE; i++)
                datagram[i]=data.data[i];
}

unsigned char deformPacket(unsigned char *datagram, char *message, Partner *partner){
	Data data;
	newData(&data);
/*
	printf("KEY : ");
        for (int i=0; i<DATA_SIZE;i++)
                printf("%02x",partner->key[i]);
        printf("\n");
	printf("XORED DATA : ");
        for (int i=0; i<DATA_SIZE;i++)
                printf("%02x",datagram[i]);
        printf("\n");
*/
        for (int i=0; i<DATA_SIZE;i++)
		data.data[i]=datagram[i];
	decryptData(&data,partner);
/*
	printf("DATA : ");
        for (int i=0; i<DATA_SIZE;i++)
                printf("%02x",data.data[i]);
        printf("\n");
*/
	for (int i=0; i<MESSAGE_SIZE; i++)
		data.message[i]=data.data[i];
	for (int i=0; i< NONCE_SIZE; i++)
		data.nonce[i]=data.data[i+MESSAGE_SIZE];
	data.flags=data.data[MESSAGE_SIZE + NONCE_SIZE];
	for (int i=0; i< HASH_SIZE; i++)
		data.hash[i]=data.data[i+MESSAGE_SIZE+NONCE_SIZE+FLAGS_SIZE];
/*
	printf("HASH : ");
	for (int i=0; i< HASH_SIZE; i++)
		printf("%02x",data.hash[i]);
	printf("\n");
*/
	if(verifyData(&data)){
		if(!data.flags){
			for(int i=0; i<MESSAGE_SIZE; i++)
				message[i]=data.message[i];
		}
		else{
			nextKey(partner);
			update(partner);
/*
	printf("NEXT KEY : ");
        for (int i=0; i<DATA_SIZE;i++)
                printf("%02x",partner->key[i]);
        printf("\n");
*/
//// ACK
		}
	}
	else{
/*
		printf("Trying to decrypt with previous key\n");
*/
		unsigned char curKey[KEY_SIZE];
		for(int i=0;i<DATA_SIZE;i++)
			curKey[i]=partner->key[i];
		for(int i=0;i<DATA_SIZE;i++)
			partner->key[i]=partner->prevKey[i];
/*
	printf("KEY : ");
        for (int i=0; i<DATA_SIZE;i++)
                printf("%02x",partner->key[i]);
        printf("\n");
	printf("XORED DATA : ");
        for (int i=0; i<DATA_SIZE;i++)
                printf("%02x",datagram[i]);
        printf("\n");
*/
        for (int i=0; i<DATA_SIZE;i++)
		data.data[i]=datagram[i];
	decryptData(&data,partner);
/*
	printf("DATA : ");
        for (int i=0; i<DATA_SIZE;i++)
                printf("%02x",data.data[i]);
        printf("\n");
*/
        for (int i=0; i<MESSAGE_SIZE; i++)
                data.message[i]=data.data[i];
        for (int i=0; i< NONCE_SIZE; i++)
                data.nonce[i]=data.data[i+MESSAGE_SIZE];
        data.flags=data.data[MESSAGE_SIZE + NONCE_SIZE];
        for (int i=0; i< HASH_SIZE; i++)
                data.hash[i]=data.data[i+MESSAGE_SIZE+NONCE_SIZE+FLAGS_SIZE];
/*
        printf("HASH : ");
        for (int i=0; i< HASH_SIZE; i++)
                printf("%02x",data.hash[i]);
        printf("\n");
*/
	if(verifyData(&data)){
		if(!data.flags){
			for(int i=0; i<MESSAGE_SIZE; i++)
				message[i]=data.message[i];
			for(int i=0; i<KEY_SIZE; i++)
				partner->key[i]=curKey[i];
///			nextKey(partner);
///nextKeyagain ?
//update need ?
			update(partner);
			return RECODE;
//// REACK
		}
//no else?
		else{
/*			nextKey(partner);
			update(partner);
	printf("NEXT KEY : ");
        for (int i=0; i<DATA_SIZE;i++)
                printf("%02x",partner->key[i]);
        printf("\n");
//// ACK
*/
		return PREV_CODE;
		}
	}
	else
		return BAD_CODE;
	}
	return data.flags;
}
