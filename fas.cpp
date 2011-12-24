#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include "fas_tournament.h"

using namespace fas_tournament;

int main(int argc, char **argv){
  srand(time(NULL) ^ getpid());
  FILE *argf = NULL;

  if(argc > 1){
    if(argc > 2){
      fprintf(stderr, "Usage: fas [inputfile]\n");
      exit(1);
    }

    argf = fopen(argv[1], "r");
    if(!argf){
      fprintf(stderr, "Unable to open file %s for reading\n", argv[1]);
      exit(1);
    }
  } else {
    argf = stdin;
  }

	tournament *t = read_tournament(argf);

  size_t n = t->size;
  size_t *items = optimal_ordering(t);

	printf("Score: %f\n", score_fas_tournament(t, n, items));
	printf("Lower bound on best score: %f\n", best_score_lower_bound(t, n, items));
	printf("Optimal ordering:");

  size_t i = 0;
  size_t next_boundary = condorcet_boundary_from(t, n, items, i);

  for(;;){
    size_t next_i = tie_starting_from(t, n, items, i);

    if(next_i > i + 1){
      printf(" [");
      for(size_t j = i; j < next_i; j++){
        if(j > i) printf(" ");
        printf("%lu", items[j]);
      }
      printf("]");
    } else {
      printf(" %lu", items[i]);
    }

    if(next_i == n) break;

    i = next_i;
    if(i > next_boundary){
      printf(" ||");
      next_boundary = condorcet_boundary_from(t, n, items, i);
    }
	}
  printf("\n");

  free(items);
	del_tournament(t);

	return 0;
}
