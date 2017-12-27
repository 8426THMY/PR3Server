#ifndef pr3_json_h
#define pr3_json_h


#include <stdlib.h>

#include "../shared/pr3_payload.h"


unsigned char jsonObjectValid(const char *str, const size_t strLength);
void jsonReadObject(payload *pl, char *str, const size_t strLength);
void jsonCreateObject(const payload *response, char **str, size_t *strLength);


#endif