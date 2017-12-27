#ifndef pr3_payload_h
#define pr3_payload_h


#include <stdlib.h>

#include "../../shared/vector.h"


typedef struct payloadVar {
	char *name;
	size_t nameLength;

	char *value;
	size_t valueLength;
} payloadVar;

typedef struct payload {
	vector vars;
} payload;


void payloadVarInit(payloadVar *plv);
void payloadVarRemove(payloadVar *plv);

void payloadInit(payload *pl);
void payloadAddVar(payload *pl, const char *name, const size_t nameLength, const char *value, const size_t valueLength);
payloadVar *payloadFindVar(const payload *pl, const char *varName, const size_t varLength);
void payloadRemove(payload *pl);


#endif