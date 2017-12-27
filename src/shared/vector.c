#include "vector.h"


#include <string.h>


void vectorInit(vector *vec){
	vec->data = malloc(sizeof(void *));
	vec->capacity = 1;
	vec->size = 0;
}

void vectorResize(vector *vec, const size_t capacity){
	if(vec->capacity != capacity){
		void **tempData = realloc(vec->data, capacity * sizeof(void *));

		if(tempData != NULL){
			vec->data = tempData;
			vec->capacity = capacity;
		}
	}
}

void vectorAdd(vector *vec, void *data, const size_t bytes){
	void *tempPointer = malloc(bytes);
	if(tempPointer != NULL){
		if(vec->size == vec->capacity){
			vectorResize(vec, vec->capacity * 2);
		}
		vec->data[vec->size] = tempPointer;
		memcpy(vec->data[vec->size], data, bytes);
		++vec->size;
	}
}

void vectorRemove(vector *vec, const size_t pos){
	if(pos < vec->size){
		free(vec->data[pos]);
		--vec->size;

		size_t i;
		for(i = pos; i < vec->size; ++i){
			vec->data[i] = vec->data[i + 1];
		}
	}
}

void *vectorGet(const vector *vec, const size_t pos){
	if(pos < vec->size){
		return(vec->data[pos]);
	}

	return(NULL);
}

void vectorSet(vector *vec, const size_t pos, void *data, const size_t bytes){
	if(pos < vec->size){
		void *tempData = realloc(vec->data[pos], bytes);

		if(tempData != NULL){
			vec->data[pos] = tempData;
			memcpy(vec->data[pos], data, bytes);
		}
	}
}

void vectorClear(vector *vec){
	if(vec->data != NULL){
		size_t i;
		for(i = 0; i < vec->size; ++i){
			free(vec->data[i]);
		}
		free(vec->data);

		vec->data = NULL;
	}
	vec->capacity = 0;
	vec->size = 0;
}