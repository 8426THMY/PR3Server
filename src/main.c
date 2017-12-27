#include <stdio.h>

#include "server/socketShared.h"
#include "server/socketServer.h"
#include "server/socketTCP.h"

#include "pr3/http/pr3_http.h"
#include "pr3/game/pr3_game.h"


int main(int argc, char *argv[]){
	/** Check the gameID for each incoming message and don't forget that you'll need to store the dataRequestID for when you respond! **/
	if(serverSetup()){
		socketServer httpServer, gameServer;

		if(httpLoadServer(&httpServer, SOCK_STREAM, IPPROTO_TCP) && gameLoadServer(&gameServer, SOCK_STREAM, IPPROTO_TCP)){
			unsigned char running = 1;
			while(running){
				running = serverListenTCP(&httpServer) && serverListenTCP(&gameServer);
			}

			serverCloseTCP(&httpServer);
			serverCloseTCP(&gameServer);
		}
    }
    serverCleanup();


	puts("\n\nPress enter to exit.");
	getc(stdin);


    return(1);
}

/*
When a user logs into the authentication server (properly, that is) it stores
it somewhere that the game servers can access it.

Also, when a client sends us something we can't handle, we should probably send
back an empty object with "write_num" so that it doesn't get out of sync.

PR3Server.exe
auth
	server1
		Pengstah
			maps
			blocks
			user.txt
				//password hash goes here
				8426 //id
		Delphinoid
			maps
			blocks
			user.txt
				//password hash goes here
				7249 //id
	server2
		SomeUser
			maps
			blocks
			user.txt
				//password hash goes here
				1234 //id
game
	server1
		8426
			user.txt
				//rank
				//hatArray
				//headArray
				//bodyArray
				//feetArray
				//hat
				//head
				//body
				//feet
				//speed
				//accel
				//jump
				//expBonus
*/