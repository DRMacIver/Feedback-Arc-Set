#include <time.h>
#include <unistd.h>

#include "tournament.h"
#include "fas_tournament.h"

int main(){
  srand(time(NULL) ^ getpid());
	tournament *t = read_tournament(stdin);

	fas_tournament *ft = run_fas_tournament(t);

	printf("Score: %f\n", ft->score);
	printf("Optimal ordering:");
	for(size_t i = 0; i < ft->results; i++){
		printf(" %lu", ft->optimal_ordering[i]);
	}
  printf("\n");

	del_fas_tournament(ft);
	del_tournament(t);

	return 0;
}
