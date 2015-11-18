#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hasher.h"
#include "xor.h"

#define KEY_SIZE 64
#define PARTNERS "partners"
#define FLAGS_SIZE 1

typedef struct Partner Partner;
struct Partner{
	int number;
	char name[KEY_SIZE];
	char ip[KEY_SIZE];
//KEY_SIZE in file
	char flags;
//KEY_SIZE in file
        unsigned char key[KEY_SIZE];
        unsigned char prevKey[KEY_SIZE];
        unsigned char prevData[KEY_SIZE];
};

int partnersNumber(){
	FILE *file = fopen(PARTNERS, "rb");
        if (file==NULL){
                printf("!!! Cant get partners!\n");
		return -1;
	}
	fseek(file, 0, SEEK_END);
	int bytes = ftell(file);
	fclose(file);
	file=NULL;
	return bytes/KEY_SIZE/6;
}

int getPartner(Partner *partner, int number){
	FILE *file = fopen(PARTNERS,"rb");
        if (file==NULL){
                printf("!!! Cant get partners!\n");
		return 1;
	}
	fseek(file,number*KEY_SIZE*6,SEEK_SET);
	fread(partner->name,KEY_SIZE,1,file);
	fclose(file);
	file=NULL;
	return 0;
}

int setPartner(Partner *partner, int number){
	partner->number=number;
	FILE *file = fopen(PARTNERS,"rb");
        if (file==NULL){
                printf("!!! Cant get partners!\n");
		return 1;
	}
	fseek(file,number*KEY_SIZE*6,SEEK_SET);
	fread(partner->name,KEY_SIZE,1,file);
	fread(partner->ip,KEY_SIZE,1,file);
	fseek(file,KEY_SIZE-KEY_SIZE,SEEK_CUR);
	char flags[FLAGS_SIZE];
	fread(flags,FLAGS_SIZE,1,file);
	partner->flags=flags[0];
	fseek(file,KEY_SIZE-FLAGS_SIZE,SEEK_CUR);
	fread(partner->key,KEY_SIZE,1,file);
	fread(partner->prevKey,KEY_SIZE,1,file);
	fread(partner->prevData,KEY_SIZE,1,file);
	fclose(file);
	file=NULL;
	return 0;
}

void nextKey(Partner *partner){
/*
	for(int i=0; i<KEY_SIZE; i++)
		partner->prevKey[i]=partner->key[i];
	hash("whirlpool", partner->prevKey, partner->key);
*/
	unsigned char curKey[KEY_SIZE];
	for(int i=0; i<KEY_SIZE; i++)
		curKey[i]=partner->key[i];
	xor(curKey,partner->prevKey);
	for(int i=0; i<KEY_SIZE; i++)
		partner->prevKey[i]=partner->key[i];
//hash for pevKey ?
	hash("whirlpool",curKey,partner->key);
	
}

int update(Partner *partner){
	FILE *file = fopen(PARTNERS,"rb+");
        if (file==NULL){
                printf("!!! Cant get partners!\n");
		return 1;
	}
	fseek(file,partner->number*KEY_SIZE*6,SEEK_SET);
	fwrite(partner->name,KEY_SIZE,1,file);
	fwrite(partner->ip,KEY_SIZE,1,file);
	fseek(file,KEY_SIZE-KEY_SIZE,SEEK_CUR);
	char flags[FLAGS_SIZE];
	flags[0]=partner->flags;
	fwrite(flags,FLAGS_SIZE,1,file);
	fseek(file,KEY_SIZE-FLAGS_SIZE,SEEK_CUR);
	fwrite(partner->key,KEY_SIZE,1,file);
	fwrite(partner->prevKey,KEY_SIZE,1,file);
	fwrite(partner->prevData,KEY_SIZE,1,file);
	fclose(file);
	file=NULL;
	return 0;
}

/*
int delete(Partner *partner){
	FILE *file = fopen(PARTNERS,"rb+");
        if (file==NULL){
                printf("!!! Cant get partners!\n");
		return 1;
	}
	fseek(file,partner->number*KEY_SIZE*6,SEEK_SET);
	int position=ftello(file);
//rewrite to current from next, truncate end-record
	ftruncate(fileno(file),position);
	fclose(file);
	file=NULL;
	return 0;
}
*/

int add(Partner *partner){
	partner->number=partnersNumber()+1;
	FILE *file = fopen(PARTNERS,"ab");
        if (file==NULL){
                printf("!!! Cant add partners!\n");
		return 1;
	}
//	fseek(file,0,SEEK_END);
	fwrite(partner->name,KEY_SIZE,1,file);
	fwrite(partner->ip,KEY_SIZE,1,file);
	partner->flags=0;
	for(int i=0; i<2*KEY_SIZE-KEY_SIZE+FLAGS_SIZE-1; i++)
		fputc(0,file);
	for(int i=0;i<KEY_SIZE;i++)
		partner->key[i]=0;
	fwrite(partner->key,KEY_SIZE,1,file);
	for(int i=0;i<KEY_SIZE;i++)
		partner->prevKey[i]=partner->key[i];
	fwrite(partner->prevKey,KEY_SIZE,1,file);
	for(int i=0;i<KEY_SIZE;i++)
		partner->prevData[i]=0;
	fwrite(partner->prevData,KEY_SIZE,1,file);
	fclose(file);
	file=NULL;
	return 0;
}
