#include "pr3_crypt.h"


#include <stdlib.h>
#include <ctype.h>
#include <string.h>


//Forward-declare our helper functions!
static void decryptBlocks(cryptor *crypt, vector *output);
static void base64Decode(const char *str, const size_t strLength, vector *output);


//Forward-declare our static variables!
const static char *base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=\0";


void cryptorInit(cryptor *crypt){
	vectorInit(&crypt->iv);
	AESInit(&crypt->AES);
}

void cryptorSetIV(cryptor *crypt, const char *str, const size_t strLength){
	vectorClear(&crypt->iv);
	vectorInit(&crypt->iv);
	base64Decode(str, strLength, &crypt->iv);
}

void cryptorDecrypt(cryptor *crypt, const char *str, const size_t strLength, char **outStr, size_t *outLength){
	vector outVec;
	vectorInit(&outVec);
	base64Decode(str, strLength, &outVec);

	decryptBlocks(crypt, &outVec);

	*outStr = realloc(*outStr, outVec.size + 1);
	char tempChar;
	//Save the decrypted string in outStr!
	for(*outLength = 0; *outLength < outVec.size; ++(*outLength)){
		tempChar = *((char *)vectorGet(&outVec, *outLength));
		(*outStr)[*outLength] = tempChar;

		//If we've read a null terminator, it's time to stop!
		if(tempChar == '\0'){
			break;
		}
	}
	(*outStr)[*outLength] = '\0';

	vectorClear(&outVec);
}

void cryptorRemove(cryptor *crypt){
	vectorClear(&crypt->iv);
	AESRemove(&crypt->AES);
}


static void decryptBlocks(cryptor *crypt, vector *output){
	vector tempIV;
	vectorInit(&tempIV);
	size_t a, b;
	for(a = 0; a < crypt->iv.size; ++a){
		vectorAdd(&tempIV, (char *)vectorGet(&crypt->iv, a), sizeof(char));
	}

	char vecVal;
	char tempArray[16];

	for(a = 0; a < output->size; a += sizeof(tempArray)){
		for(b = 0; b < sizeof(tempArray); ++b){
			if(a + b < output->size){
				tempArray[b] = *((char *)vectorGet(output, a + b));
			}else{
				tempArray[b] = 0;
			}
		}

		AESDecrypt(&crypt->AES, a, output);

		for(b = 0; b < sizeof(tempArray) && a + b < output->size; ++b){
			vecVal = *((char *)vectorGet(output, a + b)) ^ *((char *)vectorGet(&tempIV, b));
			vectorSet(output, a + b, &vecVal, sizeof(vecVal));
		}

		for(b = 0; b < sizeof(tempArray); ++b){
			if(b > tempIV.size){
				vectorAdd(&tempIV, &tempArray[b], sizeof(*tempArray));
			}else{
				vectorSet(&tempIV, b, &tempArray[b], sizeof(*tempArray));
			}
		}
	}

	vectorClear(&tempIV);
}

static void base64Decode(const char *str, const size_t strLength, vector *output){
	char tempArray1[4];
	char tempArray2[3];
	char *tempPos;

	size_t a, b;
	for(a = 0; a < strLength; a += 4){
		for(b = 0; b < 4 && a + b < strLength; ++b){
			tempPos = strchr(base64Chars, str[a + b]);
			if(tempPos != NULL){
				tempArray1[b] = tempPos - base64Chars;
			}else{
				tempArray1[b] = -1;
			}
		}

		tempArray2[0] = (tempArray1[0] << 2) + ((tempArray1[1] & 48) >> 4);
		tempArray2[1] = ((tempArray1[1] & 15) << 4) + ((tempArray1[2] & 60) >> 2);
		tempArray2[2] = ((tempArray1[2] & 3) << 6) + tempArray1[3];

		for(b = 0; b < sizeof(tempArray2); ++b){
			if(tempArray1[b + 1] == 64){
				break;
			}

			vectorAdd(output, &tempArray2[b], sizeof(*tempArray2));
		}
	}
}