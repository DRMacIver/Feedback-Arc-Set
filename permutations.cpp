#include "permutations.h"
#include <assert.h>

// Permutations of arrays of size_t
namespace permutations{

static inline void swap(size_t *x, size_t *y){
	if(x == y) return;
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

	for(size_t s = l; s < length; s++){
		if(data[k]  < data[s]) l = s;
	}

	swap(data + k, data + l);

	reverse(data + k + 1, data + length - 1);

	return k;
}

inline static size_t saturate(size_t v){
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v |= v >> 32;
	return v;
}

size_t random_number(size_t n){
	size_t mask = saturate(n);

	size_t result;

	for(;;){
		result = rand() & mask;
		if(result < n) return result;
	}
}

void shuffle(size_t length, size_t *data){
	for(size_t k = length - 1; k > 0; k--){
		size_t j = random_number(k+1);
		assert(j <= k);
		swap(data + j, data + k);
	}
}

void generate_shuffled_range(size_t length, size_t *data){
	data[0] = 0;
	for(size_t i = 1; i < length; i++){
		size_t j = random_number(i + 1);
		if(j < i) data[i] = data[j];
		data[j] = i;
	}
}

}
