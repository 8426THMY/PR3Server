#ifndef pr3_gameRespond_h
#define pr3_gameRespond_h


#include "../../server/socketServer.h"

#include "../shared/pr3_payload.h"


void gameRespondDomainPolicy(const socketServer *server, const size_t clientID);
void gameRespondLogin(const payload *message, const socketServer *server, const size_t clientID);
void gameRespondConfirmConnection(const payload *message, const socketServer *server, const size_t clientID);
void gameRespondPing(const payload *message, const socketServer *server, const size_t clientID);
void gameRespondManageVars(const payload *message, const socketServer *server, const size_t clientID);


#endif