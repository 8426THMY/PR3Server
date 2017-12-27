#ifndef pr3_aes_h
#define pr3_aes_h


#include <stdio.h>

#include "../../../shared/vector.h"


typedef struct AESKey {
	char *key;
	size_t keyLength;
	size_t plainKeyLength;
} AESKey;


void AESInit(AESKey *AES);
void AESSetKey(AESKey *AES, const char *newKey, const size_t newKeyLength);
void AESDecrypt(AESKey *AES, const size_t offset, vector *output);
void AESRemove(AESKey *AES);


#endif