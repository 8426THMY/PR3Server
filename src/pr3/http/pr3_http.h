#ifndef pr3_http_h
#define pr3_http_h


#include "../../server/socketServer.h"


unsigned char httpLoadServer(socketServer *server, const int type, const int protocol);


#endif