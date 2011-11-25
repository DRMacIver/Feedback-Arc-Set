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

	int num_trials = 50;

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

int main(){
  srand(time(NULL) ^ getpid());

  size_t n = 500;
  run_tests("unstructured", random_tournament(n));
  run_tests("voting", random_tournament_from_voting(n));

  return 0;
}
