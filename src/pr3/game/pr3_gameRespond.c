#include "pr3_gameRespond.h"


#include <stdlib.h>
#include <string.h>

#include "../../server/socketTCP.h"

#include "../xml/pr3_xml.h"
#include "../json/pr3_json.h"


//Forward-declare our helper functions!
static void sendResponse(const payload *message, const payload *response, const socketServer *server, const size_t clientID);


//Send the domain policy!
void gameRespondDomainPolicy(const socketServer *server, const size_t clientID){
	char *tempMsg = NULL;
	size_t tempMsgLength;
	xmlGetDomainPolicy(&tempMsg, &tempMsgLength);

	serverSendTCP(server, clientID, tempMsg, tempMsgLength + 1);

	free(tempMsg);
}

//Let the client log in!
void gameRespondLogin(const payload *message, const socketServer *server, const size_t clientID){
	payload response;
	payloadInit(&response);

	//Add our variables to the response payload!
	payloadAddVar(&response, "type", 4, "loginSuccess", 12);
	payloadAddVar(&response, "userName", 8, "Pengstah", 8);
	payloadAddVar(&response, "vars", 4, "{\"rank\":0,"
										"\"hatArray\":[1],"
										"\"hatColor\":16737792,"
										"\"headArray\":[1,2,3],"
										"\"headColor\":16737792,"
										"\"bodyArray\":[1,2,3],"
										"\"bodyColor\":16737792,"
										"\"feetArray\":[1,2,3],"
										"\"feetColor\":16737792}", 168);
	/*payloadAddVar(&response, "vars", 4, "{\"rank\":8426,"
                                        "\"hat\":8,\"hatArray\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19],"
                                        "\"head\":8,\"headArray\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25],"
                                        "\"body\":8,\"bodyArray\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25],"
                                        "\"feet\":8,\"feetArray\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25]}", 349);*/

	//Send the response!
	sendResponse(message, &response, server, clientID);

	//Remove our response payload!
	payloadRemove(&response);
}

//Send a connection confirmation!
void gameRespondConfirmConnection(const payload *message, const socketServer *server, const size_t clientID){
	payload response;
	payloadInit(&response);

	//Add our variables to the response payload!
	payloadAddVar(&response, "type", 4, "ready", 5);

	//Send the response!
	sendResponse(message, &response, server, clientID);

	//Remove our response payload!
	payloadRemove(&response);
}

//Send the client's ping!
void gameRespondPing(const payload *message, const socketServer *server, const size_t clientID){
	//{"t":"ping","time":1501381002384,"write_num":3}\4
	payloadVar *timeVar = payloadFindVar(message, "time", 4);
	if(timeVar != NULL){
		payload response;
		payloadInit(&response);

		//Add our variables to the response payload!
		payloadAddVar(&response, "type", 4, "ping", 4);
		payloadAddVar(&response, "time", 4, timeVar->value, timeVar->valueLength);

		//Send the response!
		//sendResponse(message, &response, server, clientID);

		//Remove our response payload!
		payloadRemove(&response);
	}
}

//Manage some variables...?
void gameRespondManageVars(const payload *message, const socketServer *server, const size_t clientID){
	/** Future note: Colours are modulo 16777215 and parts should be stored in order. **/
	//{"t":"mv","user_vars":"*","action":"get","location":"user","id":"0","write_num":2}\4
	//{"action":"get","write_num":3,"id":"0","location":"user","user_vars":["rank","hatArray","headArray","bodyArray","feetArray"],"t":"mv"}\4
	//Check whose variables we have to manage!
	payloadVar *locVar = payloadFindVar(message, "location", 8);
	if(locVar != NULL){
		//Check which action to perform on them!
		payloadVar *actVar = payloadFindVar(message, "action", 6);
		if(actVar != NULL){
			payload response;
			payloadInit(&response);

			if(strcmp(locVar->value, "user") == 0){
				if(strcmp(actVar->value, "get") == 0){
					//Add our variables to the response payload!
					payloadAddVar(&response, "type", 4, "receiveUserVars", 15);
					payloadAddVar(&response, "userName", 8, "Pengstah", 8);
					payloadAddVar(&response, "vars", 4, "{\"rank\":0,"
														"\"hatArray\":[1],"
														"\"hatColor\":16737792,"
														"\"headArray\":[1,2,3],"
														"\"headColor\":16737792,"
														"\"bodyArray\":[1,2,3],"
														"\"bodyColor\":16737792,"
														"\"feetArray\":[1,2,3],"
														"\"feetColor\":16737792}", 168);
					/*payloadAddVar(&response, "vars", 4, "{\"guestName\":\"Pengstah\","
														"\"rank\":8426,"
														"\"hat\":8,\"hatArray\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19],"
														"\"head\":8,\"headArray\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25],"
														"\"body\":8,\"bodyArray\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25],"
														"\"feet\":8,\"feetArray\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25]}", 372);*/

					//Send the response!
					sendResponse(message, &response, server, clientID);
				}
			}

			//Remove our response payload!
			payloadRemove(&response);
		}
	}
}


//Format a response payload as a JSON object and send it!
static void sendResponse(const payload *message, const payload *response, const socketServer *server, const size_t clientID){
	payloadVar *idVar = payloadFindVar(message, "write_num", 9);
	if(idVar != NULL){
		//Add the request I.D. to the beginning of our response!
		size_t tempMsgLength = idVar->valueLength + 1;
		char *tempMsg = malloc(tempMsgLength);
		memcpy(tempMsg, idVar->value, idVar->valueLength);
		strcpy(tempMsg + idVar->valueLength, " ");

		//Now convert the response to a JSON string!
		jsonCreateObject(response, &tempMsg, &tempMsgLength);

		//Add a EOT and a null terminator!
		tempMsg = realloc(tempMsg, tempMsgLength + 2);
		strcpy(tempMsg + tempMsgLength, "\4\0");

		//Send the response!
		serverSendTCP(server, clientID, tempMsg, tempMsgLength + 1);

		//Free our message!
		free(tempMsg);
	}
}