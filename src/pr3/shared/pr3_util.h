#ifndef pr3_util_h
#define pr3_util_h


//These indicate the maximum number of characters
//required to store a number of each type.
#define LONG_MAX_CHARS 11
//The number of extra characters we need for file buffers.
//Includes the null-terminator and an overflow-check character.
#define EXTRA_CHARS 2


#include <stdio.h>


int strncasecmp(const char *str1, const char *str2, size_t num);
size_t ltostr(long num, char *str);
size_t getTokenLength(const char *str, const size_t strLength, const char *delims);
void loadConfig(char *configPath, char **ip, size_t *ipLength, unsigned short *port, size_t *bufferSize);


#endif