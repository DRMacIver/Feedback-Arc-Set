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
	for(size_t i = 0; i < ft->results; i++){
		printf(" %lu", ft->optimal_ordering[i]);
	}
  printf("\n");

	del_fas_tournament(ft);
	del_tournament(t);

	return 0;
}
