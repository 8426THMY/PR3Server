#include "pr3_httpPayload.h"


#include <stdlib.h>
#include <string.h>

#include "../../shared/vector.h"

#include "../shared/pr3_util.h"
#include "../shared/pr3_payload.h"
#include "pr3_httpRequest.h"
#include "crypt/pr3_crypt.h"
#include "crypt/pr3_aes.h"


//Forward-declare our helper functions!
static void urlDecode(const char *str, const size_t strLength, char **outStr, size_t *outLength);


//Store all of the payload's variables!
void httpPayloadRead(payload *pl, const httpRequest *request){
	if(request->payloadStart != NULL){
		char *varStart = request->payloadStart;
		size_t varLength;
		char *tokStart;
		size_t tokLength;

		char *tempName = NULL;
		size_t tempNameLength = 0;
		char *tempValue = NULL;
		size_t tempValueLength = 0;

		while(varStart < request->payloadStart + request->payloadLength){
			//Find the parameter's delimiter!
			varLength = getTokenLength(varStart, request->payloadLength - (varStart - request->payloadStart), "&");

			if(varLength > 0){
				//Find the value separator!
				tokStart = varStart;
				tokLength = getTokenLength(varStart, varLength, "=");
				if(tokLength < varLength){
					size_t i;
					for(i = 0; i < 2; ++i){
						//Store the variable's name!
						if(i == 0){
							tempName = tokStart;
							tempNameLength = tokLength;

						//Store the variable's value!
						}else{
							//Make sure we decode it first, though!
							urlDecode(tokStart, tokLength, &tempValue, &tempValueLength);
						}

						//Move to the next token!
						++tokLength;
						tokStart += tokLength;
						tokLength = varLength - tokLength;
					}

					//Add the variable to the payload!
					payloadAddVar(pl, tempName, tempNameLength, tempValue, tempValueLength);
				}
			}

			varStart += varLength + 1;
		}

		//We only need to free the temporary value.
		if(tempValue != NULL){
			free(tempValue);
		}
	}
}

//Decrypt all of the payload data!
void httpPayloadDecrypt(payload *pl, const payloadVar *iv){
	if(iv != NULL){
		//Set up our 'cryptor!
		cryptor crypt;
		cryptorInit(&crypt);
		cryptorSetIV(&crypt, iv->value, iv->valueLength);
		AESSetKey(&crypt.AES, "012345678910ABCD", 16);

		payloadVar *tempVar;

		size_t i;
		for(i = 0; i < pl->vars.size; ++i){
			tempVar = (payloadVar *)vectorGet(&pl->vars, i);
			//We don't want to decrypt the variable if it's the I.D., I.V. or the debug indicator.
			if(tempVar != iv && strcmp(tempVar->name, "dataRequestID") != 0 && strcmp(tempVar->name, "debug") != 0){
				//Decrypt the variable!
				cryptorDecrypt(&crypt, tempVar->value, tempVar->valueLength, &tempVar->value, &tempVar->valueLength);
			}
		}

		cryptorRemove(&crypt);
	}
}


//Decodes a URL-encoded string.
static void urlDecode(const char *str, const size_t strLength, char **outStr, size_t *outLength){
	*outStr = realloc(*outStr, strLength + 1);

	size_t offset = 0;
	for(*outLength = 0; *outLength + offset < strLength; ++(*outLength)){
		(*outStr)[*outLength] = str[*outLength + offset];

		if(str[*outLength + offset] == '%'){
			char hexNum[3];
			memcpy(hexNum, str + *outLength + offset + 1, 2 * sizeof(*hexNum));
			hexNum[2] = '\0';
			(*outStr)[*outLength] = strtol(hexNum, NULL, 16);

			offset += 2;
		}
	}

	*outStr = realloc(*outStr, *outLength + 1);
	(*outStr)[*outLength] = '\0';
}