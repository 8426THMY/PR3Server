#include "pr3_payload.h"


#include <string.h>


void payloadVarInit(payloadVar *plv){
	plv->name = NULL;
	plv->nameLength = 0;

	plv->value = NULL;
	plv->valueLength = 0;
}

void payloadVarRemove(payloadVar *plv){
	if(plv->name != NULL){
		free(plv->name);
	}
	if(plv->value != NULL){
		free(plv->value);
	}
}

void payloadInit(payload *pl){
	vectorInit(&pl->vars);
}

//Add a new variable to the payload!
void payloadAddVar(payload *pl, const char *name, const size_t nameLength, const char *value, const size_t valueLength){
	payloadVar tempVar;

	//Set the variable's name!
	tempVar.name = malloc(nameLength + 1);
	memcpy(tempVar.name, name, nameLength);
	tempVar.name[nameLength] = '\0';
	tempVar.nameLength = nameLength;

	//Set the variable's value!
	tempVar.value = malloc(valueLength + 1);
	memcpy(tempVar.value, value, valueLength);
	tempVar.value[valueLength] = '\0';
	tempVar.valueLength = valueLength;

	//Add the variable to our vector!
	vectorAdd(&pl->vars, &tempVar, sizeof(tempVar));
}

//Find a specific variable!
payloadVar *payloadFindVar(const payload *pl, const char *varName, const size_t varLength){
	payloadVar *tempVar;

	size_t i;
	for(i = 0; i < pl->vars.size; ++i){
		tempVar = (payloadVar *)vectorGet(&pl->vars, i);
		if(strcmp(tempVar->name, varName) == 0){
			return(tempVar);
		}
	}

	return(NULL);
}

void payloadRemove(payload *pl){
	size_t i;
	for(i = 0; i < pl->vars.size; ++i){
		payloadVarRemove((payloadVar *)vectorGet(&pl->vars, i));
	}

	vectorClear(&pl->vars);
}