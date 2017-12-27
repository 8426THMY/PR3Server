#include "pr3_json.h"


#include <string.h>

#include "../shared/pr3_util.h"


//Check if the payload is valid!
unsigned char jsonObjectValid(const char *str, const size_t strLength){
	if(*str == '{' && str[strLength - 2] == '}' && str[strLength - 1] == '\4'){
		return(1);
	}

	return(0);
}

//Store all of the payload's variables!
void jsonReadObject(payload *pl, char *str, const size_t strLength){
	char *varStart = &str[1];
	size_t varLength;
	char *tokStart;
	size_t tokLength;

	while(varStart < &str[strLength]){
		//Find the parameter's delimiter!
		varLength = getTokenLength(varStart, strLength - (varStart - str), ",}\4{");

		if(varLength > 0){
			//Find the value separator!
			tokStart = varStart;
			tokLength = getTokenLength(tokStart, varLength, ":");
			if(tokLength < varLength){
				char *tempName = NULL;
				size_t tempNameLength = 0;
				char *tempValue = NULL;
				size_t tempValueLength = 0;

				size_t i;
				for(i = 0; i < 2; ++i){
					//If the token contains quotes, cut them off!
					if(*tokStart == '"'){
						++tokStart;
						--tokLength;
						tempValueLength = tokLength - 1;

					//Otherwise, store the whole thing!
					}else{
						tempValueLength = tokLength;
					}

					//Store the variable's name!
					if(i == 0){
						tempName = tokStart;
						tempNameLength = tempValueLength;

					//Store the variable's value!
					}else{
						tempValue = tokStart;
					}

					//Move to the next token!
					tokLength++;
					tokStart += tokLength;
					if(tokStart < varStart + varLength){
						tokLength = varStart + varLength - tokStart;
					}else{
						tokLength = 0;
					}
				}

				//Add the variable to the payload!
				payloadAddVar(pl, tempName, tempNameLength, tempValue, tempValueLength);
			}
		}

		varStart += varLength + 1;
	}
}

//Create a JSON object string from a payload!
void jsonCreateObject(const payload *response, char **str, size_t *strLength){
	//Store our current offset in the response string!
	size_t offset = *strLength;
	//This will be the size of our response string!
	*strLength += 1;
	//We don't want to keep calling realloc, so we allocate more memory than we need.
	size_t memSize = *strLength;
	//Add the opening brace to the beginning of the string!
	*str = realloc(*str, memSize);
	(*str)[offset++] = '{';

	payloadVar *tempVar;
	char *valueEnd;

	size_t i;
	//Add all of our response's variables to the string!
	for(i = 0; i < response->vars.size; ++i){
		tempVar = (payloadVar *)vectorGet(&response->vars, i);
		//Get the new size of the string!
		//We add three for the name's quotes and the delimiter!
		*strLength += tempVar->nameLength + 3 + tempVar->valueLength;

		//If the value is not a number, an array or an object, add quotes!
		if(*tempVar->value != '{' && *tempVar->value != '['){
			strtod(tempVar->value, &valueEnd);
			if(*valueEnd != '\0'){
				*strLength += 2;
			}
		}else{
			valueEnd = tempVar->value + tempVar->valueLength;
		}
		//If this isn't the last value, add a comma!
		if(i < response->vars.size - 1){
			++(*strLength);
		}

		//Adjust the amount of memory we've allocated if we have to!
		if(memSize < *strLength){
			memSize = *strLength * 2;
			*str = realloc(*str, memSize);
		}


		//Add the variable name to the string!
		(*str)[offset++] = '"';
		memcpy(*str + offset, tempVar->name, tempVar->nameLength);
		offset += tempVar->nameLength;
		(*str)[offset++] = '"';
		(*str)[offset++] = ':';
		//Add the variable value to the string!
		if(*valueEnd != '\0'){
			(*str)[offset++] = '"';
			memcpy(*str + offset, tempVar->value, tempVar->valueLength);
			offset += tempVar->valueLength;
			(*str)[offset++] = '"';
		}else{
			memcpy(*str + offset, tempVar->value, tempVar->valueLength);
			offset += tempVar->valueLength;
		}
		//Add a comma if we have to!
		if(i < response->vars.size - 1){
			(*str)[offset++] = ',';
		}
	}

	++(*strLength);
	*str = realloc(*str, *strLength + 1);

	//Add the end of the string!
	(*str)[offset++] = '}';
	(*str)[offset] = '\0';
}