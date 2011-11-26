#include "tournament.h"
#include "fas_tournament.h"
#include "permutations.h"
#include <time.h>
#include <unistd.h>


void run_tests(char *label, tournament *t){
	size_t n = t->size;

	size_t *trial_ordering = malloc(n * sizeof(size_t));

	for(size_t i = 0; i < n; i++){
		trial_ordering[i] = i;
	}

	int num_trials = 100;

	double best = 0.0;
	double total = 0.0;

	for(int i = 0; i < num_trials; i++){
		shuffle(n, trial_ordering);

		double score = score_fas_tournament(t, n, trial_ordering);
		if(score > best) best = score;
		total += score;
	}

	fas_tournament *ft = run_fas_tournament(t);

  printf("%s\t%lu\t%f\t%f\t%f\n", label, t->size, best, ft->score, (100 * (ft->score - best) / best));

  del_tournament(t);
	del_fas_tournament(ft);
}

int main(int argc, char **argv){
  srand(time(NULL) ^ getpid());

  if(argc > 1){
    for(int i = 1; i < argc; i++){
      FILE *f = fopen(argv[i], "r");
    
      if(!f){
        fprintf(stderr, "Unable to open %s for reading\n", argv[i]);
        exit(1);
      } else {
        tournament *t = read_tournament(f);
        fclose(f);
        run_tests(argv[i], t);
      }
    }
  } else {
    tournament *t = read_tournament(stdin);
    run_tests("stdin", t);
  }

  return 0;
}
