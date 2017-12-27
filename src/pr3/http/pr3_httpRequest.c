#include "pr3_httpRequest.h"


#include <string.h>

#include "../../server/socketTCP.h"

#include "../shared/pr3_util.h"


//Forward-declare our helper functions!
static unsigned char getFormat(const httpRequest *request);


void httpRequestInit(httpRequest *request){
	request->methodStart = NULL;
	request->methodLength = 0;

	request->targetStart = NULL;
	request->targetLength = 0;

	request->responseFormatStart = NULL;
	request->responseFormatLength = 0;

	request->payloadFormatStart = NULL;
	request->payloadFormatLength = 0;

	request->payloadStart = NULL;
	request->payloadLength = 0;
}

//Check if a request is valid, storing the type and target.
//Returns 0 on failure or the character we reached if successful.
size_t httpRequestValid(httpRequest *request, char *str, const size_t strLength){
	char *tokStart = str;
	size_t tokLength;
	size_t numToks = 0;

	while(tokStart < &str[strLength]){
		tokLength = getTokenLength(tokStart, strLength - (tokStart - str), " \r\n");

		//Check the type.
		if(numToks == 0){
			if(tokLength == 4 && memcmp(tokStart, "POST", 4) == 0){
				request->methodStart = tokStart;
				request->methodLength = 4;
			}else if(tokLength == 3 && memcmp(tokStart, "GET", 3) == 0){
				request->methodStart = tokStart;
				request->methodLength = 3;
			}else{
				return(0);
			}

		//Store the target.
		}else if(numToks == 1){
			request->targetStart = tokStart;
			request->targetLength = tokLength;

		//Check the version.
		}else{
			if(tokLength == 8 && memcmp(tokStart, "HTTP/1.1", 8) == 0){
				//Make sure we skip trailing "\r\n" characters!
				if(*(tokStart + tokLength) == '\r'){
					++tokLength;
				}
				if(*(tokStart + tokLength) == '\n'){
					++tokLength;
				}

				return(tokStart + tokLength - str);
			}else{
				break;
			}
		}

		++numToks;

		tokStart += tokLength + 1;
		//Try and treat \r\n as a single character.
		if(*tokStart == '\n' && *(tokStart - 1) == '\r'){
			++tokStart;
		}
	}


	return(0);
}

//Fills a httpRequest structure with information pertaining to a request.
void httpRequestDetails(httpRequest *request, char *str, const size_t strLength){
	char *tokStart = str;
	size_t tokLength;

	while(tokStart < str + strLength){
		tokLength = getTokenLength(tokStart, strLength - (tokStart - str), "\r\n");

		//Store the expected response format.
		if(tokLength > 8 && strncasecmp(tokStart + 6, ": ", 2) == 0){
			request->responseFormatStart = tokStart + 8;
			request->responseFormatLength = tokLength - 8;

		//If this is a post request, store the data format and payload.
		}else if(request->methodLength == 4){
			if(tokLength > 14 && strncasecmp(tokStart, "content", 7) == 0){
				if(strncasecmp(tokStart + 7, "-type: ", 7) == 0){
					request->payloadFormatStart = tokStart + 14;
					request->payloadFormatLength = tokLength - 14;
				}else if(tokLength > 16 && strncasecmp(tokStart + 7, "-length: ", 9) == 0){
					request->payloadLength = strtol(tokStart + 16, NULL, 10);
				}
			}else if(tokLength == 0){
				request->payloadStart = tokStart;

				//Make sure we skip trailing "\r\n" characters!
				if(*request->payloadStart == '\r'){
					++request->payloadStart;
				}
				if(*request->payloadStart == '\n'){
					++request->payloadStart;
				}

				return;
			}
		}

		tokStart += tokLength + 1;
		//Try and treat \r\n as a single character.
		if(*tokStart == '\n' && *(tokStart - 1) == '\r'){
			++tokStart;
		}
	}
}

void httpRequestRespond(const httpRequest *request, const char *str, const size_t strLength, const socketServer *server, const size_t clientID){
	if(strLength > 0){
		const unsigned char format = getFormat(request);
		char conLen[LONG_MAX_CHARS + 1];
		const size_t conLenSize = ltostr(strLength, conLen);

		char *message;
		size_t messageLength = 48 + conLenSize + strLength;

		//If the format is 1 or 0, use "text/xml"!
		if(format <= 1){
			messageLength += 10;
			message = malloc(messageLength);
			strcpy(message, "HTTP/1.1 200 OK\nContent-Length: ");
			strcpy(message + 32, conLen);
			strcpy(message + 32 + conLenSize, "\nContent-Type: text/xml\n\n");

		//Otherwise, the format is 2, so use "application/xml"!
		}else{
			messageLength += 17;
			message = malloc(messageLength);
			strcpy(message, "HTTP/1.1 200 OK\nContent-Length: ");
			strcpy(message + 32, conLen);
			strcpy(message + 32 + conLenSize, "\nContent-Type: application/xml\n\n");
		}

		//Now add the string and the null terminator!
		--messageLength;
		strcpy(message + messageLength - strLength, str);
		message[messageLength] = '\0';

		//Finally, send the message!
		serverSendTCP(server, clientID, message, messageLength + 1);

		free(message);
	}
}


//Return which text format the response should use!
static unsigned char getFormat(const httpRequest *request){
	//Format type key:
	//0 = unsupported format requested
	//1 = text/xml || text/* || */*
	//2 = application/xml || application/*

	if(request->responseFormatStart != NULL){
		char *tokStart = request->responseFormatStart;
		size_t tokLength;
		size_t tempLength = request->responseFormatLength;

		while(tempLength > 0){
			tokLength = getTokenLength(tokStart, tempLength, ", ");

			//Check for "application/xml" or "application/*".
			if((tokLength > 11 && memcmp(tokStart, "application", 11) == 0) &&
			   ((tokLength >= 15 && memcmp(tokStart + 12, "xml", 3) == 0) ||
			   (tokLength >= 13 && *(tokStart + 12) == '*'))){

				return(2);

			//Check for "text/xml" or "text/*".
			}else if((tokLength > 4 && memcmp(tokStart, "text", 4) == 0) &&
			         ((tokLength >= 8 && memcmp(tokStart + 5, "xml", 3) == 0) ||
			         (tokLength >= 6 && *(tokStart + 5) == '*'))){

				return(1);

			//Check for "*/xml" or "*/*". In this case, use "text/xml"!
			}else if(tokLength > 1 && *(tokStart) == '*' &&
			         ((tokLength >= 5 && memcmp(tokStart + 2, "xml", 3) == 0) ||
			         (tokLength >= 3 && *(tokStart + 2) == '*'))){

				return(1);
			}

			++tokLength;
			//Don't let tempLength go below 0. It's unsigned!
			if(tempLength > tokLength){
				tokStart += tokLength;
				tempLength -= tokLength;
				//Try and treat \r\n as a single character.
				if(*tokStart == '\n' && *(tokStart - 1) == '\r'){
					++tokStart;
					--tempLength;
				}
			}else{
				tempLength = 0;
			}
		}
	}else{
		//Use text/xml if no format was specified.
		return(1);
	}


	//Uh oh, we don't support any of the formats listed!
	return(0);
}