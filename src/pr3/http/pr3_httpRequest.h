#ifndef pr3_httpRequest_h
#define pr3_httpRequest_h


#include <stdlib.h>

#include "../../server/socketServer.h"


typedef struct httpRequest {
	char* methodStart;
	size_t methodLength;

	char *targetStart;
	size_t targetLength;

	char *responseFormatStart;
	size_t responseFormatLength;

	char *payloadFormatStart;
	size_t payloadFormatLength;

	char *payloadStart;
	size_t payloadLength;
} httpRequest;


void httpRequestInit(httpRequest *request);
size_t httpRequestValid(httpRequest *request, char *str, const size_t strLength);
void httpRequestDetails(httpRequest *request, char *str, const size_t strLength);
void httpRequestRespond(const httpRequest *request, const char *str, const size_t strSize, const socketServer *server, const size_t clientID);


#endif