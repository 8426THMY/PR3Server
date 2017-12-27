#ifndef pr3_xml_h
#define pr3_xml_h


#include <stdlib.h>

#include "../shared/pr3_payload.h"


void xmlReadObject(payload *pl, char *str, const size_t strLength);
void xmlCreateObject(const payload *pl, char **str, size_t *strLength);
void xmlGetDomainPolicy(char **str, size_t *strLength);
void xmlGetUserDetails(char **str, size_t *strLength);
void xmlCreateServerRow(char **str, size_t *strLength, const char *name, const unsigned short port, const char *address, const char *status);


#endif