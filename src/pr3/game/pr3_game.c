#include "pr3_game.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../server/socketTCP.h"

#include "../json/pr3_json.h"
#include "../shared/pr3_util.h"
#include "../shared/pr3_payload.h"
#include "pr3_gameRespond.h"


//Forward-declare our helper functions!
static void gameBuffer(socketServer *server, const size_t clientID);
static void gameDisconnect(socketServer *server, const size_t clientID);


unsigned char gameLoadServer(socketServer *server, const int type, const int protocol){
	char *ip = NULL;
	size_t ipLength = 0;
	unsigned short port = DEFAULT_PORT;
	size_t bufferSize = DEFAULT_BUFFER_SIZE;
	loadConfig("./config/gameServer.cfg", &ip, &ipLength, &port, &bufferSize);

	const unsigned char success = serverInit(server, type, protocol, ip, ipLength, port, bufferSize, &gameBuffer, &gameDisconnect);

	free(ip);
	return(success);
}


static void gameBuffer(socketServer *server, const size_t clientID){
	unsigned char success = 0;

	//Check if this is a valid game message!
	if(jsonObjectValid(server->buffer, server->bufferLength)){
		payload message;
		payloadInit(&message);
		//Read the message!
		jsonReadObject(&message, server->buffer, server->bufferLength);

		//Find the message type and the request I.D.!
		payloadVar *typeVar = ((typeVar = payloadFindVar(&message, "t", 1)) != NULL) ? typeVar : payloadFindVar(&message, "type", 4);
		if(typeVar != NULL){
			//A registered user wants to log in!
			if(strcmp(typeVar->value, "login") == 0){
				gameRespondLogin(&message, server, clientID);
				success = 1;

			//A guest wants to connect!
			}else if(strcmp(typeVar->value, "confirm_connection") == 0){
				gameRespondConfirmConnection(&message, server, clientID);
				success = 1;

			//Send the client a response to their ping!
			}else if(strcmp(typeVar->value, "ping") == 0){
				gameRespondPing(&message, server, clientID);
				success = 1;

			//The client wants us to do something with their user variables!
			}else if(strcmp(typeVar->value, "mv") == 0){
				printf("%s\n", server->buffer);
				gameRespondManageVars(&message, server, clientID);
				success = 1;
			}
		}

		payloadRemove(&message);

	//If the client is requesting the policy file, send it to him!
	}else if(strcmp(server->buffer, "<policy-file-request/>") == 0){
		gameRespondDomainPolicy(server, clientID);
		success = 1;
	}


	//Looks like the client has sent something we don't recognize.
	if(!success){
		printf("Client #%u has sent something we can't handle at the moment:\n"
		       "%s\n\n", clientID, server->buffer);
	}
}

static void gameDisconnect(socketServer *server, const size_t clientID){
	printf("Client #%u has been disconnected from the game server.\n", clientID);
}