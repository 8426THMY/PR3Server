#include "pr3_httpRespond.h"


#define PR3_NO_ERROR 0
#define PR3_INVALID_PAYLOAD 1
#define PR3_USER_EXISTS 2
#define PR3_INVALID_USER 3
#define PR3_WRITE_FAILURE 4
#define PR3_DATA_TOO_LONG 5

#define MD5_LENGTH 32


#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

#include "../xml/pr3_xml.h"
#include "../shared/pr3_util.h"
#include "pr3_httpPayload.h"


//Forward-declare our helper functions!
static long int getNextUserID();
static unsigned char validateUser(const payloadVar *username, const payloadVar *password, long int *userID);
static void logEmIn(const httpRequest *request, const socketServer *server, const size_t clientID, const long userID, const unsigned char error);
static void getServers(const httpRequest *request, const socketServer *server, const size_t clientID);
static void getLoginToken(const httpRequest *request, const socketServer *server, const size_t clientID);


//Send the domain policy!
void httpRespondDomainPolicy(const httpRequest *request, const socketServer *server, const size_t clientID){
	char *tempMsg = NULL;
	size_t tempMsgLength;
	xmlGetDomainPolicy(&tempMsg, &tempMsgLength);

	httpRequestRespond(request, tempMsg, tempMsgLength, server, clientID);

	free(tempMsg);
}

//Create an account for this user if we can and log them in!
void httpRespondCreateUser(const httpRequest *request, const socketServer *server, const size_t clientID){
	unsigned char error = 0;
	long int userID = -1;


	payload message;
	payloadInit(&message);

	//Read the payload!
	httpPayloadRead(&message, request);
	//Find the variable containing the encryption IV and decrypt the payload!
	httpPayloadDecrypt(&message, payloadFindVar(&message, "usertype", 8));

	//This payload will contain all of the new user's details!
	payload userDetails;
	payloadInit(&userDetails);

	//Fill it up, cap'n!
	payloadVar *tempVar = payloadFindVar(&message, "parameters", 10);
	xmlReadObject(&userDetails, tempVar->value, tempVar->valueLength);
	if(userDetails.vars.size == 5){
		payloadVar *username;
		//Find the username and password.
		if((username = payloadFindVar(&userDetails, "UserName", 8)) != NULL && (tempVar = payloadFindVar(&userDetails, "PasswordHash", 12)) != NULL){
			//Now generate the file path!
			char *userFilePath = malloc(17 + username->valueLength);
			strcpy(userFilePath, ".\\auth\\");
			memcpy(userFilePath + 7, username->value, username->valueLength);
			strcpy(userFilePath + 7 + username->valueLength, "\\user.txt");

			FILE *userFile = fopen(userFilePath, "r");
			//If the account doesn't exist, create it and log them in!
			if(userFile == NULL){
				//Create the necessary folders!
				char *curSlash = strchr(userFilePath + 2, '\\');
				while(curSlash != NULL){
					*curSlash = '\0';
					mkdir(userFilePath);
					*curSlash = '\\';
					curSlash = strchr(curSlash + 1, '\\');
				}


				//Get their password and generate a unique I.D. for them!
				if((userID = getNextUserID() - 1) >= 0){
					userFile = fopen(userFilePath, "w");
					if(userFile != NULL){
						//Get the current date!
						const time_t curTime = time(NULL);
						const struct tm *curDate = localtime(&curTime);

						//Now write it all to the file!
						fprintf(userFile, "%s\n%li\n%u-%u-%u", tempVar->value, userID, 1900 + curDate->tm_year, curDate->tm_mon + 1, curDate->tm_mday);


						printf("Successfully registered %s!\n\n", username->value);
						fclose(userFile);
					}else{
						perror("Failed to create user.txt");
						printf("Path: %s\n\n", userFilePath);

						error = PR3_WRITE_FAILURE;
					}
				}

			//If we can open this file, the account probably already exists!
			}else{
				fclose(userFile);

				error = PR3_USER_EXISTS;
			}

			free(userFilePath);
		}else{
			error = PR3_INVALID_PAYLOAD;
		}
	}else{
		error = PR3_INVALID_PAYLOAD;
	}
	payloadRemove(&userDetails);
	payloadRemove(&message);


	//Send a response to the client!
	logEmIn(request, server, clientID, userID, error);
}

//Check if the client's login details are valid and log them in if they are!
void httpRespondLogin(const httpRequest *request, const socketServer *server, const size_t clientID){
	unsigned char error;
	long int userID = -1;


	payload message;
	payloadInit(&message);

	//Read the payload!
	httpPayloadRead(&message, request);
	//Find the variable containing the encryption IV and decrypt the payload!
	httpPayloadDecrypt(&message, payloadFindVar(&message, "storedProcID", 12));

	//Make sure the user's name and password are legitimate!
	error = validateUser(payloadFindVar(&message, "userName", 8), payloadFindVar(&message, "passwordHash", 12), &userID);

	payloadRemove(&message);


	//Send a response to the client!
	logEmIn(request, server, clientID, userID, error);
}

//The client wants to access some data!
void httpRespondDataAccess(const httpRequest *request, const socketServer *server, const size_t clientID){
	payload message;
	payloadInit(&message);

	//Read the payload!
	httpPayloadRead(&message, request);
	//Find the variable containing the encryption IV and decrypt the payload!
	httpPayloadDecrypt(&message, payloadFindVar(&message, "storedProcID", 12));

	//Find the procedure name and request I.D.!
	payloadVar *tempVar = payloadFindVar(&message, "storedProcedureName", 19);
	if(tempVar != NULL){
		//Send a list of game servers to the client!
		if(strcmp(tempVar->value, "GetServers2") == 0){
			getServers(request, server, clientID);

		//Send the client their server I.D.!
		}else if(strcmp(tempVar->value, "GetLoginToken2") == 0){
			getLoginToken(request, server, clientID);

		//Looks like we don't recognize this procedure.
		}else{
			printf("Client #%u has sent an unrecognized dataaccess2 request!\nProcedure name: %s\n\n", clientID, tempVar->value);
			size_t i;
			for(i = 0; i < message.vars.size; ++i){
				tempVar = (payloadVar *)vectorGet(&message.vars, i);
				printf("%s - %s\n", tempVar->name, tempVar->value);
			}
			puts("\n\n");
		}
	}

	payloadRemove(&message);
}


//Get the next unique user I.D.!
static long int getNextUserID(){
	long int userID = 0;


	char *curFilePath = malloc(8);
	strcpy(curFilePath, ".\\auth\\");
	DIR *serverDir = opendir(curFilePath);

	struct stat curFileStat;
	struct dirent *curFile;
	//Count the number of folders!
	while((curFile = readdir(serverDir)) != NULL){
		//Make sure we don't count the two dot folders!
		if(strcmp(curFile->d_name, ".") != 0 && strcmp(curFile->d_name, "..") != 0){
			curFilePath = realloc(curFilePath, 8 + curFile->d_namlen);
			strcpy(curFilePath + 7, curFile->d_name);
			curFilePath[curFile->d_namlen + 7] = '\0';

			//If this file is a folder, increment the I.D.!
			if(stat(curFilePath, &curFileStat) == 0 && S_ISDIR(curFileStat.st_mode)){
				++userID;
			}
		}
	}


	return(userID);
}

static unsigned char validateUser(const payloadVar *username, const payloadVar *password, long int *userID){
	unsigned char error = PR3_NO_ERROR;


	if(username != NULL){
		//Make sure the name isn't empty!
		if(username->valueLength != 0){
			if(password != NULL){
				//Now generate the file path!
				char *userFilePath = malloc(17 + username->valueLength);
				strcpy(userFilePath, ".\\auth\\");
				memcpy(userFilePath + 7, username->value, username->valueLength);
				strcpy(userFilePath + 7 + username->valueLength, "\\user.txt");

				FILE *userFile = fopen(userFilePath, "r");
				//If the account exists, check the password!
				if(userFile != NULL){
					//MD5 hashes are always 32 characters. We need another
					//two for the newline character and the null terminator.
					//We also store the user's I.D. in here, but it should
					//only be 11 characters big at the most.
					char tempBuffer[MD5_LENGTH + EXTRA_CHARS];
					tempBuffer[MD5_LENGTH] = '\n';

					//The password hash is on the first line of the file.
					fgets(tempBuffer, MD5_LENGTH + EXTRA_CHARS, userFile);
					if(tempBuffer[MD5_LENGTH] == '\n'){
						//Remove the trailing newline character so "strcmp" works.
						tempBuffer[MD5_LENGTH] = '\0';
						//If the password is correct, get the user's I.D.!
						if(strcmp(password->value, tempBuffer) == 0){
							tempBuffer[LONG_MAX_CHARS + 1] = '\n';
							//The user's I.D. should be on the next line.
							fgets(tempBuffer, LONG_MAX_CHARS + EXTRA_CHARS, userFile);
							if(tempBuffer[LONG_MAX_CHARS + 1] == '\n'){
								*userID = strtoul(tempBuffer, NULL, 10);
							}else{
								error = PR3_INVALID_USER;
							}
						}else{
							error = PR3_INVALID_USER;
						}

					//If the second last character is no longer a newline character, we read too many characters!
					}else{
						error = PR3_DATA_TOO_LONG;
					}

					fclose(userFile);

				//If we can't open this file, the account doesn't exist!
				}else{
					error = PR3_INVALID_USER;
				}

				free(userFilePath);
			}else{
				error = PR3_INVALID_PAYLOAD;
			}

		//If the username is empty, a guest is trying to log in!
		//In that case, we should set the userID to -1.
		}else{
			*userID = -1;
		}
	}else{
		error = PR3_INVALID_PAYLOAD;
	}


	return(error);
}

//Send a login response to the client!
static void logEmIn(const httpRequest *request, const socketServer *server, const size_t clientID, const long userID, const unsigned char error){
	payload response;
	payloadInit(&response);

	//We need at least 12 characters to store the user I.D.!
	char *responseString = malloc(12);
	size_t responseStringLength;

	//Add the data we're sending to the payload!
	payloadAddVar(&response, "userId", 6, responseString, ltostr(userID, responseString));
	payloadAddVar(&response, "Success", 7, responseString, ltostr(error == 0, responseString));
	switch(error){
		case PR3_NO_ERROR:
			payloadAddVar(&response, "Error", 5, "", 0);
		break;
		case PR3_INVALID_PAYLOAD:
			payloadAddVar(&response, "Error", 5, "Invalid payload.", 16);
		break;
		case PR3_USER_EXISTS:
			payloadAddVar(&response, "Error", 5, "User already exists.", 20);
		break;
		case PR3_INVALID_USER:
			payloadAddVar(&response, "Error", 5, "Incorrect username or password.", 31);
		break;
		default:
			payloadAddVar(&response, "Error", 5, "Miscellaneous server-side error.", 32);
		break;
	}

	//Convert it to an XML string!
	responseStringLength = 0;
	xmlCreateObject(&response, &responseString, &responseStringLength);


	//Add "Params" tags around the rest of the data!
	payloadRemove(&response);
	payloadInit(&response);
	payloadAddVar(&response, "Params", 6, responseString, responseStringLength);

	responseStringLength = 0;
    xmlCreateObject(&response, &responseString, &responseStringLength);


    //Send the response!
	httpRequestRespond(request, responseString, responseStringLength, server, clientID);


	//Free our data!
	free(responseString);
	payloadRemove(&response);
}

//Send the client a list of servers!
static void getServers(const httpRequest *request, const socketServer *server, const size_t clientID){
	payload response;
	payloadInit(&response);
	//Add the request I.D. so the client can find the right callback function!
	payloadAddVar(&response, "DataRequestID", 13, request->targetStart + 17, request->targetLength - 17);

	char *tempMsg = NULL;
	size_t tempMsgLength;


	//This is the number of servers in the list.
	payloadAddVar(&response, "NumRows", 7, "1", 1);

	//Add the server to the list!
	xmlCreateServerRow(&tempMsg, &tempMsgLength, "Tommy's Server", 8426, "127.0.0.1", "online");
	payloadAddVar(&response, "Row", 3, tempMsg, tempMsgLength);

	//Convert the payload to a string!
	tempMsgLength = 0;
	xmlCreateObject(&response, &tempMsg, &tempMsgLength);

	//Add "Params" tags around the rest of the data!
	payloadRemove(&response);
	payloadInit(&response);
	payloadAddVar(&response, "Params", 6, tempMsg, tempMsgLength);

	tempMsgLength = 0;
	xmlCreateObject(&response, &tempMsg, &tempMsgLength);


	//Now send the response!
	httpRequestRespond(request, tempMsg, tempMsgLength, server, clientID);


	//Free our temporary data!
	if(tempMsg != NULL){
		free(tempMsg);
	}

	payloadRemove(&response);
}

//Send the client's their server I.D.!
static void getLoginToken(const httpRequest *request, const socketServer *server, const size_t clientID){
	payload response;
	payloadInit(&response);
	//Add the request I.D. so the client can find the right callback function!
	payloadAddVar(&response, "DataRequestID", 13, request->targetStart + 17, request->targetLength - 17);

	char clientIDStr[LONG_MAX_CHARS + 1];
	const size_t clientIDStrLength = ltostr(clientID, clientIDStr);

	size_t tempMsgLength = 27 + clientIDStrLength;
	char *tempMsg = malloc(tempMsgLength + 1);

	//Add the login token to the payload!
	strcpy(tempMsg, "<login_token>");
	strcpy(tempMsg + 13, clientIDStr);
	strcpy(tempMsg + 13 + clientIDStrLength, "</login_token>");
	payloadAddVar(&response, "Row", 3, tempMsg, tempMsgLength);

	//Convert the payload to a string!
	tempMsgLength = 0;
	xmlCreateObject(&response, &tempMsg, &tempMsgLength);

	//Add "Params" tags around the rest of the data!
	payloadRemove(&response);
	payloadInit(&response);
	payloadAddVar(&response, "Params", 6, tempMsg, tempMsgLength);

	tempMsgLength = 0;
	xmlCreateObject(&response, &tempMsg, &tempMsgLength);


	//Now send the response!
	httpRequestRespond(request, tempMsg, tempMsgLength, server, clientID);


	//Free our temporary data!
	if(tempMsg != NULL){
		free(tempMsg);
	}

	payloadRemove(&response);
}