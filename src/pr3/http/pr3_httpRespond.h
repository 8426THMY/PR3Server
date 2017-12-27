#ifndef pr3_httpRespond_h
#define pr3_httpRespond_h


#include <stdlib.h>

#include "../../server/socketServer.h"

#include "pr3_httpRequest.h"


void httpRespondDomainPolicy(const httpRequest *request, const socketServer *server, const size_t clientID);
void httpRespondCreateUser(const httpRequest *request, const socketServer *server, const size_t clientID);
void httpRespondLogin(const httpRequest *request, const socketServer *server, const size_t clientID);
void httpRespondDataAccess(const httpRequest *request, const socketServer *server, const size_t clientID);


#endif