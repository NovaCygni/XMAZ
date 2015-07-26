#ifndef DEF_PARTNER
#define DEF_PARTNER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hasher.h"

#define KEY_SIZE 64
#define PARTNERS "partners"

typedef struct Partner Partner;
struct Partner{
	int number;
	char name[KEY_SIZE];
	char ip[KEY_SIZE];
	char flags;
        unsigned char key[KEY_SIZE];
        unsigned char prevKey[KEY_SIZE];
        unsigned char prevData[KEY_SIZE];
};

int partnersNumber();
int getPartner(Partner *partner, int number);
int setPartner(Partner *partner, int number);
void nextKey(Partner *partner);
int update(Partner *partner);
//void delete(Partner *partner);
int add(Partner *partner);

#endif
