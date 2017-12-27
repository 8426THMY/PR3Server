#include "pr3_util.h"


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>


//Forward-declare our helper functions!
static char *readLineFile(FILE *file, char *line, size_t *lineLength);


//Case-insensitive version of strncmp.
int strncasecmp(const char *str1, const char *str2, size_t num){
	if(num == 0){
		return(0);
	}

	while(num-- != 0 && tolower(*str1) == tolower(*str2)){
		if(num == 0 || *str1 == '\0' || *str2 == '\0'){
			break;
		}
		++str1;
		++str2;
	}

	return(tolower(*(unsigned char *)str1) - tolower(*(unsigned char *)str2));
}

//Convert an integer to a string.
//We assume that "str" points to an array with at least (LONG_MAX_CHARS + 1) characters.
size_t ltostr(long num, char *str){
	//Special case for when num is equal to 0!
	if(num == 0){
		*str++ = '0';
		*str = '\0';

		return(1);
	}


	size_t length = 0;
	char *curPos = str;

	//Check for a minus sign.
	if(num < 0){
		*curPos = '-';
		num = -num;
	}else{
		*curPos = '\0';
	}

	//Add the digits backwards, starting at the end of the array.
	curPos += 10;
	while(num > 0){
		*curPos-- = '0' + num % 10;
		num /= 10;
		++length;
	}

	//Now copy them over to the front!
	if(*str != '-'){
		memcpy(str, curPos + 1, length);
	}else{
		if(length < 10){
			memcpy(str + 1, curPos + 1, length);
		}
		++length;
	}

	//Add a null terminator and we're set!
	str[length] = '\0';


	return(length);
}

//Get the length of a token!
size_t getTokenLength(const char *str, const size_t strLength, const char *delims){
	const char *tokStart = str;

	//Keep looping until we get to the end of the string or a null terminator!
	while(*tokStart != '\0' && tokStart - str < strLength){
		const char *curDelim = delims;
		while(*curDelim != '\0'){
			if(*tokStart == *curDelim){
				return(tokStart - str);
			}
			++curDelim;
		}
		++tokStart;
	}


	return(strLength);
}

void loadConfig(char *configPath, char **ip, size_t *ipLength, unsigned short *port, size_t *bufferSize){
	FILE *configFile = fopen(configPath, "rb");
	if(configFile != NULL){
		char lineBuffer[1024];
		char *line;
		size_t lineLength;

		unsigned int tempNum;
		char *endPos;


		while((line = readLineFile(configFile, lineBuffer, &lineLength)) != NULL){
			//Set the I.P.
			if(strncmp(line, "ip = ", 5) == 0){
				*ipLength = lineLength - 5;
				*ip = realloc(*ip, *ipLength + 1);
				if(*ip != NULL){
					memcpy(*ip, line + 5, *ipLength);
					(*ip)[*ipLength] = '\0';
				}

			//Set the port.
			}else if(strncmp(line, "port = ", 7) == 0){
				tempNum = strtol(line + 7, &endPos, 10);
				//Make sure it's valid!
				if(*endPos == '\0' && errno != ERANGE && tempNum < (USHRT_MAX * 2)){
					*port = tempNum;
				}

			//Set the buffer size.
			}else if(strncmp(line, "buff = ", 7) == 0){
				tempNum = strtol(line + 7, &endPos, 10);
				//Make sure it's valid!
				if(*endPos == '\0' && errno != ERANGE && tempNum > 0){
					*bufferSize = tempNum;
				}
			}
		}
	}else{
		printf("Unable to open config file!\n"
		       "Path: %s\n\n", configPath);
	}
	fclose(configFile);
}


//Read a line from a file, removing any unwanted stuff!
static char *readLineFile(FILE *file, char *line, size_t *lineLength){
	line = fgets(line, 1024, file);
	if(line != NULL){
		*lineLength = strlen(line);

		//Remove comments.
		char *tempPos = strstr(line, "//");
		if(tempPos != NULL){
			*lineLength -= *lineLength - (tempPos - line);
		}

		//"Remove" whitespace characters from the beginning of the line!
		tempPos = &line[*lineLength];
		while(line < tempPos && isspace(*line)){
			++line;
		}
		*lineLength = tempPos - line;

		//"Remove" whitespace characters from the end of the line!
		while(*lineLength > 0 && isspace(line[*lineLength - 1])){
			--*lineLength;
		}

		line[*lineLength] = '\0';
	}


	return(line);
}