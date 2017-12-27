#ifndef serverSettings_h
#define serverSettings_h


#define SERVER_MAX_SOCKETS 201
#define SERVER_USE_POLL
//Note: This should be variable, and preferably an input to socketListen.
//If no one is sending anything, it should block until it's time to update.
#define SERVER_POLL_TIMEOUT 0
#define SERVER_SOCKET_TIMEOUT 30000


#endif