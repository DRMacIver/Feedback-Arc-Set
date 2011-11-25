#include "permutations.h"

// Permutations of arrays of size_t

void swap(size_t *x, size_t *y){
	size_t z = *x;
	*x = *y;
	*y = z;
}

void reverse(size_t *s, size_t *e){
	while(s < e){
		swap(s, e);
		s++;
		e--;
	}	
}

// Returns index of first index altered or length if no changes
size_t next_permutation(size_t length, size_t *data){
	if(length <= 1) return length;

	// Guaranteed not to overflow by above
	size_t k = length - 2;

	while(data[k] >= data[k+1]){
		if(k > 0) k--;
		// We are in decreasing order and thus are done.
		else return length;
	}

	// Now we have data[k] < data[k+1]
	// We want to find the largest index l such that data[k] < data[l]
	size_t l = k + 1;

	for(int s = l; s < length; s++){
		if(data[k]  < data[s]) l = s;
	}

	swap(data + k, data + l);

	reverse(data + k + 1, data + length - 1);

	return k;
}

