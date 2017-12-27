#include "pr3_xml.h"


#include <string.h>

#include "../shared/pr3_util.h"


//Read an XML object (not in a particularly good way, mind you).
void xmlReadObject(payload *pl, char *str, const size_t strLength){
	char *varStart = str;
	size_t varLength;

	while(varStart < &str[strLength]){
		char *tempName = NULL;
		size_t tempNameLength = 0;
		char *tempValue = NULL;
		size_t tempValueLength = 0;

		//Find the end of the line!
		varLength = getTokenLength(varStart, strLength - (varStart - str), "\r\n");

		tempName = strchr(varStart, '<');
		//Find the opening tag and get the tag's name!
		if(tempName != NULL){
			++tempName;

			//Make sure this isn't a closing tag!
			if(tempName < varStart + varLength && *tempName != '/'){
				//Find the end of the opening tag!
				tempNameLength = getTokenLength(tempName, varLength - (tempName - varStart), ">");

				//Get the variable's value!
				if(tempName + tempNameLength + 1 < varStart + varLength){
					tempValue = tempName + tempNameLength + 1;
					tempValueLength = getTokenLength(tempValue, varLength - (tempValue - varStart), "<");

					//Add the variable to the payload!
					payloadAddVar(pl, tempName, tempNameLength, tempValue, tempValueLength);
				}else if(tempName[tempNameLength - 1] == '/'){
					--tempNameLength;

					//Add the variable to the payload!
					payloadAddVar(pl, tempName, tempNameLength, tempValue, tempValueLength);
				}
			}
		}

		varStart += varLength + 1;
		//Try and treat \r\n as a single character.
		if(*varStart == '\n' && *(varStart - 1) == '\r'){
			++varStart;
		}
	}
}

//Create an XML object string from a payload!
void xmlCreateObject(const payload *pl, char **str, size_t *strLength){
	//Store our current offset in the response string!
	size_t offset = *strLength;
	//We don't want to keep calling realloc, so we allocate more memory than we need.
	size_t memSize = *strLength;

	payloadVar *tempVar;

	size_t i;
	for(i = 0; i < pl->vars.size; ++i){
		tempVar = (payloadVar *)vectorGet(&pl->vars, i);
		*strLength += tempVar->nameLength * 2 + 5 + tempVar->valueLength;
		//Adjust the amount of memory we've allocated if we have to!
		if(memSize < *strLength){
			memSize = *strLength * 2;
			*str = realloc(*str, memSize);
		}

		//Add it to the string!
		(*str)[offset++] = '<';
		memcpy(*str + offset, tempVar->name, tempVar->nameLength);
		offset += tempVar->nameLength;
		(*str)[offset++] = '>';
		memcpy(*str + offset, tempVar->value, tempVar->valueLength);
		offset += tempVar->valueLength;
		(*str)[offset++] = '<';
		(*str)[offset++] = '/';
		memcpy(*str + offset, tempVar->name, tempVar->nameLength);
		offset += tempVar->nameLength;
		(*str)[offset++] = '>';
	}

	//Append a null terminator!
	*str = realloc(*str, *strLength + 1);
	(*str)[*strLength] = '\0';
}

//Set "str" to the domain policy as an XML string!
void xmlGetDomainPolicy(char **str, size_t *strLength){
	*str = realloc(*str, 109);
	strcpy(*str, "<?xml version=\"1.0\"?><cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\"/></cross-domain-policy>");
	*strLength = 109;
}

//Set "str" to the server list as an XML string!
/*void xmlGetServerList(char **str, size_t *strLength){
	*str = realloc(*str, 189);
	strcpy(*str, "<Params><DataRequestID>0</DataRequestID><NumRows>1</NumRows><Row><server_name>Tommy's Server</server_name><port>8426</port><address>127.0.0.1</address><status>online</status></Row></Params>");
	*strLength = 189;
}*/


//Create a new server row string!
void xmlCreateServerRow(char **str, size_t *strLength, const char *name, const unsigned short port, const char *address, const char *status){
	const size_t nameLength = strlen(name);
	char portStr[LONG_MAX_CHARS + 1];
	const size_t portStrLength = ltostr(port, portStr);
	const size_t addressLength = strlen(address);
	const size_t statusLength = strlen(status);

	*strLength = 76 + nameLength + portStrLength + addressLength + statusLength;
	*str = realloc(*str, *strLength + 1);

	char *strPos = *str;
	//Add the server's name!
	strcpy(strPos, "<server_name>");
	strPos += 13;
	strcpy(strPos, name);
	strPos += nameLength;
	strcpy(strPos, "</server_name>");
	strPos += 14;
	//Add the server's port!
	strcpy(strPos, "<port>");
	strPos += 6;
	strcpy(strPos, portStr);
	strPos += portStrLength;
	strcpy(strPos, "</port>");
	strPos += 7;
	//Add the server's address!
	strcpy(strPos, "<address>");
	strPos += 9;
	strcpy(strPos, address);
	strPos += addressLength;
	strcpy(strPos, "</address>");
	strPos += 10;
	//Add the server's status!
	strcpy(strPos, "<status>");
	strPos += 8;
	strcpy(strPos, status);
	strPos += statusLength;
	strcpy(strPos, "</status>");
	strPos[9] = '\0';


	/*payload serverDeets;
	payloadAddVar(&serverDeets, "server_name", 11, name, nameLength);
	payloadAddVar(&serverDeets, "port", 4, portStr, portStrLength);
	payloadAddVar(&serverDeets, "address", 7, address, addressLength);
	payloadAddVar(&serverDeets, "status", 6, status, statusLength);*/
}