#include "tournament.h"
#include "fas_tournament.h"

int main(){
	tournament *t = read_tournament(stdin);

	fas_tournament *ft = run_fas_tournament(t);

	printf("Score: %f\n", ft->score);
	printf("Optimal ordering:\n");
	for(size_t i = 0; i < ft->results; i++){
		printf("  %lu\n", ft->optimal_ordering[i]);
	}

	del_fas_tournament(ft);
	del_tournament(t);

	return 0;
}
