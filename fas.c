#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include "tournament.h"
#include "fas_tournament.h"

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

	fas_tournament *ft = run_fas_tournament(t);

	printf("Score: %f\n", ft->score);
	printf("Optimal ordering:");

  size_t i = 0;
  size_t next_boundary = condorcet_boundary_from(t, ft->results, ft->optimal_ordering, i);

  for(;;){
    size_t next_i = tie_starting_from(t, ft->results, ft->optimal_ordering, i);

    if(next_i > i + 1){
      printf(" [");
      for(size_t j = i; j < next_i; j++){
        if(j > i) printf(" ");
        printf("%lu", ft->optimal_ordering[j]);
      }
      printf("]");
    } else {
      printf(" %lu", ft->optimal_ordering[i]);
    }

    if(next_i == ft->results) break;

    i = next_i;
    if(i >= next_boundary){
      printf(" ||");
      next_boundary = condorcet_boundary_from(t, ft->results, ft->optimal_ordering, i);
    }
	}
  printf("\n");

	del_fas_tournament(ft);
	del_tournament(t);

	return 0;
}
