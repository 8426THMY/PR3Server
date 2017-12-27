#ifndef vector_h
#define vector_h


#include <stdlib.h>


typedef struct vector {
	void **data;
	size_t capacity;
	size_t size;
} vector;


void vectorInit(vector *vec);
void vectorResize(vector *vec, const size_t capacity);
void vectorAdd(vector *vec, void *data, const size_t bytes);
void vectorRemove(vector *vec, const size_t pos);
void *vectorGet(const vector *vec, const size_t pos);
void vectorSet(vector *vec, const size_t pos, void *data, const size_t bytes);
void vectorClear(vector *vec);


#endif