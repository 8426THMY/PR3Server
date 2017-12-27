#ifndef pr3_httpPayload_h
#define pr3_httpPayload_h


#include "../shared/pr3_payload.h"
#include "pr3_httpRequest.h"


void httpPayloadRead(payload *pl, const httpRequest *request);
void httpPayloadDecrypt(payload *pl, const payloadVar *iv);


#endif