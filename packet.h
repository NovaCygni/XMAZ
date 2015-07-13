#ifndef DEF_PACKET
#define DEF_PACKET
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
#define BAD_CODE 2

void formPacket(char *message, unsigned char *datagram, Partner *partner);
unsigned char deformPacket(unsigned char *datagram, char *message, Partner *partner);
#endif
