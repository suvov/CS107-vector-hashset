#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define kInitialAllocation 4

static void VectorGrow(vector *v)
{
	v->allocLen *= 2;
	v->elems = realloc(v->elems, v->allocLen * v->elemSize);
}

void VectorNew(vector *v, int elemSize, VectorFreeFunction freefn, int initialAllocation)
{		
	assert(elemSize > 0);
	assert(initialAllocation >= 0);
	v->elemSize = elemSize;
	v->logLen = 0;
	if (initialAllocation > 0)
		v->allocLen = initialAllocation;
	else 
		v->allocLen = kInitialAllocation;
	v->elems = malloc(v->allocLen * v->elemSize);
	v->freefn = freefn;
	assert(v->elems != NULL);
}

void VectorDispose(vector *v)
{
	if(v->freefn != NULL) {
		int i;	
		for(i = 0; i < v->logLen; i++) {
			void * freeAt = (char *)v->elems + i * v->elemSize; // address of element to be freed
			v->freefn(freeAt);
		}		
	}
	free(v->elems); 
}

int VectorLength(const vector *v)
{ 
	return v->logLen;	 
}

void *VectorNth(const vector *v, int position)
{ 
	assert(position >= 0 && position <= v->logLen-1);	
	void * elem;
	elem = (char *)v->elems + position * v->elemSize;
	
	return elem; 
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
	assert(position >= 0 && position <= v->logLen-1);
	void * destAddr;
	destAddr = (char *)v->elems + position * v->elemSize;
	if(v->freefn != NULL) v->freefn(destAddr); //cleaning memory before losing access to it
	memcpy(destAddr, elemAddr, v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
	assert(position >= 0 && position <= v->logLen);
	if(v->logLen == v->allocLen) VectorGrow(v);
	void * insertAddr = (char *)v->elems + position * v->elemSize;
	void * nextAddr = (char *) insertAddr + v->elemSize;	
	int n_bytes = (v->logLen - position) * v->elemSize;
		
	memmove(nextAddr, insertAddr, n_bytes);
	memmove(insertAddr, elemAddr, v->elemSize);
	v->logLen++;
	
}

void VectorAppend(vector *v, const void *elemAddr)
{
	void * destAddr;
	if(v->logLen == v->allocLen) VectorGrow(v);
	destAddr = (char *)v->elems + v->logLen * v->elemSize;
	memcpy(destAddr, elemAddr, v->elemSize);
	v->logLen++;
}	

void VectorDelete(vector *v, int position)
{
	assert(position >= 0 && position <= v->logLen-1);
	void * delAddr = (char *)v->elems + position * v->elemSize;
	if(position == v->logLen -1 && v->freefn != NULL) v->freefn(delAddr); // easy case -- last elem			
	else{
		if(v->freefn != NULL) v->freefn(delAddr);
		const void * nextAddr = (char *) delAddr + v->elemSize;
		int n_bytes = (v->logLen - position -1)	* v->elemSize;
		memmove(delAddr, nextAddr, n_bytes);//probably i could've used memcpy here...	
	}	
	v->logLen--;	
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
	assert(compare != NULL);
	void * base = v->elems;
	int size = v->logLen;
	int elemSize = v->elemSize;
	qsort(base, size, elemSize, compare);	
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
	assert(mapFn != NULL);	
	int i;
	for(i = 0; i < v->logLen; i++) {
		void * elemAddr = v->elems + i * v->elemSize;
		mapFn(elemAddr, auxData);	
	}
}

static const int kNotFound = -1;

// lsearch function
void *srcFn (void *key, void *base, int n, int elemSize, int (*cmpFn)(void*, void*))
{
  int i;
  for(i = 0; i < n; i++) {
    void *elemAddr = (char*)base + i * elemSize;
    if(cmpFn(key, elemAddr) == 0)
      return elemAddr;
  }
  return NULL;
}


int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{ 
	assert(startIndex >= 0 && startIndex <= v->logLen-1);
	assert(searchFn != NULL && key != NULL);
	void * base = (char *)v->elems + startIndex * v->elemSize;
	int nmemb = v->logLen - startIndex;	
	int elemSize = v->elemSize;
	void * found;	
	if(isSorted) {
		found  = bsearch(key, base, nmemb, elemSize, searchFn);			
	}else{
		//const int * nmembPt = &nmemb;
		found = srcFn(key, base, nmemb, elemSize, searchFn); //<--
	}
	
	if(found != NULL) 
		return ((char *) found - (char *) v->elems) / v->elemSize; // position in vector
	else
		return kNotFound;	 
} 

