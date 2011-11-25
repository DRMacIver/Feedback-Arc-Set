#include "tournament.h"
#include "permutations.h"

int main(){
  tournament *t = random_tournament(20);

  print_tourmanent(stdout, t);

  del_tournament(t);

	printf("\n-----------------------\n");

	size_t xs[5] = {0, 1, 2, 3, 4};

	do {
		for(size_t i = 0; i < 5; i++){
			printf("%d ", (int)xs[i]);
		}
		printf("\n");
	} while(next_permutation(5, xs) < 5);

  return 0;
}
