#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert(elemSize > 0 && numBuckets > 0);
	assert(hashfn != NULL && comparefn != NULL);
	h->elemSize = elemSize;
	h->numBuckets = numBuckets;
	h->hashfn = hashfn;
	h->comparefn = comparefn;
	h->freefn = freefn;
	h->elems = malloc(h->numBuckets * sizeof(vector));
	assert(h->elems != NULL);
	int i;
	for(i = 0 ; i < h->numBuckets; i++) 
		VectorNew(&h->elems[i], h->elemSize, NULL, 0);	
}

void HashSetDispose(hashset *h)
{
	int i;
	for(i = 0; i < h->numBuckets; i++)
		VectorDispose(&h->elems[i]);
	free(h->elems);
}

int HashSetCount(const hashset *h)
{ 
	int size = 0;
	int i;
	for (i = 0; i < h->numBuckets; i++) 
		size += VectorLength(&h->elems[i]);
	
	return size; 
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert(mapfn != NULL);	
	int i;
	for(i = 0; i < h->numBuckets; i++) 
		VectorMap(&h->elems[i], mapfn, auxData);	
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	assert(elemAddr != NULL);
	int bucketNo = h->hashfn(elemAddr, h->numBuckets);
	assert(bucketNo >= 0 && bucketNo <= h->numBuckets);
	int found;
	if(VectorLength(&h->elems[bucketNo]) != 0) { // if bucket not empty
		found = VectorSearch(&h->elems[bucketNo], elemAddr, h->comparefn, 0, false); // we looking for element
		if(found >= 0) 	{
			VectorReplace(&h->elems[bucketNo], elemAddr, found); // if found, replace
		}else{
			VectorAppend(&h->elems[bucketNo], elemAddr); // if not, append		
		}
	} else {
		VectorAppend(&h->elems[bucketNo], elemAddr);// if bucket is empty, we just append	
	}	
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{ 
	assert(elemAddr != NULL);
	int bucketNo = h->hashfn(elemAddr, h->numBuckets);
	assert(bucketNo >= 0 && bucketNo <= h->numBuckets);
	if(VectorLength(&h->elems[bucketNo]) != 0) { 
		int found = VectorSearch(&h->elems[bucketNo], elemAddr, h->comparefn, 0, false);
		if(found >= 0) 
			return VectorNth(&h->elems[bucketNo], found);			
	} 
	return NULL; 	
}
