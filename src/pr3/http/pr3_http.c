#include "pr3_http.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../server/socketTCP.h"

#include "../shared/pr3_util.h"
#include "pr3_httpRequest.h"
#include "pr3_httpRespond.h"


//Forward-declare our helper functions!
static void httpBuffer(socketServer *server, const size_t clientID);
static void httpDisconnect(socketServer *server, const size_t clientID);


unsigned char httpLoadServer(socketServer *server, const int type, const int protocol){
	char *ip = NULL;
	size_t ipLength = 0;
	unsigned short port = DEFAULT_PORT;
	size_t bufferSize = DEFAULT_BUFFER_SIZE;
	loadConfig("./config/httpServer.cfg", &ip, &ipLength, &port, &bufferSize);

	const unsigned char success = serverInit(server, type, protocol, ip, ipLength, port, bufferSize, &httpBuffer, &httpDisconnect);

	free(ip);
	return(success);
}


static void httpBuffer(socketServer *server, const size_t clientID){
	unsigned char success = 0;

	httpRequest request;
	httpRequestInit(&request);
	//Check if the client is sending a HTTP request.
	size_t requestPos = httpRequestValid(&request, server->buffer, server->bufferLength);
	if(requestPos){
		//Get the request's details!
		httpRequestDetails(&request, server->buffer + requestPos, server->bufferLength - requestPos);

		//Check all the GET requests we're expecting.
		if(request.methodLength == 3){
			//The client is requesting the policy file, so give them one!
			if(request.targetLength == 16 && memcmp(request.targetStart, "/crossdomain.xml", 16) == 0){
				httpRespondDomainPolicy(&request, server, clientID);
				success = 1;
			}

		//Check all the POST requests we're expecting.
		}else if(request.methodLength == 4){
			//We can only really handle payloads in this format at the moment.
			if(request.payloadFormatStart != NULL && request.payloadFormatLength == 33 && memcmp(request.payloadFormatStart, "application/x-www-form-urlencoded", 33) == 0){
				//The user wants to create an account!
				if(request.targetLength == 23 && memcmp(request.targetStart, "/newuser/?id=CreateUser", 23) == 0){
					httpRespondCreateUser(&request, server, clientID);
					success = 1;

				//The user wants to log in!
				}else if(request.targetLength == 17 && memcmp(request.targetStart, "/login2/?id=Login", 17) == 0){
					httpRespondLogin(&request, server, clientID);
					success = 1;

				//The client wants to access some data!
				}else if(request.targetLength >= 12 && memcmp(request.targetStart, "/dataaccess2", 12) == 0){
					httpRespondDataAccess(&request, server, clientID);
					success = 1;
				}
			}
		}
	}


	//If they sent something we don't recognize, disconnect them!
	if(!success){
		printf("Client #%u has sent something we can't handle at the moment:\n"
		       "%s\n\n"
		       "Disconnecting them...\n\n", clientID, server->buffer);

		serverDisconnectTCP(server, clientID);
	}
}

static void httpDisconnect(socketServer *server, const size_t clientID){
	printf("Client #%u has been disconnected from the authentication server.\n", clientID);
}