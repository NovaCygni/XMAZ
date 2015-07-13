#ifndef DEF_PARTNER
#define DEF_PARTNER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hasher.h"

#define KEY_SIZE 64
#define PARTNERS "partners"
#define IP_SIZE 4*4


typedef struct Partner Partner;
struct Partner{
	int number;
	char name[KEY_SIZE];
	char ip[IP_SIZE];
	char flags;
        unsigned char key[KEY_SIZE];
        unsigned char prevKey[KEY_SIZE];
        unsigned char prevData[KEY_SIZE];
};

int partnersNumber();
void getPartner(Partner *partner, int number);
void setPartner(Partner *partner, int number);
void nextKey(Partner *partner);
void update(Partner *partner);
//void delete(Partner *partner);
void add(Partner *partner);

#endif
