#include "tournament.h"
#include "fas_tournament.h"
#include <time.h>
#include <unistd.h>

int main(){
  srand(time(NULL) ^ getpid());

  tournament *t = random_tournament(30);

  print_tourmanent(stdout, t);


	printf("\n-----------------------\n");

	fas_tournament *ft = run_fas_tournament(t);

	printf("Score: %f\n", ft->score);

	printf("Results: ");

	for(size_t i = 0; i < ft->results; i++){
		printf("%d ", (int)ft->optimal_ordering[i]);
	}

	printf("\n");

  del_tournament(t);
	del_fas_tournament(ft);

  return 0;
}
