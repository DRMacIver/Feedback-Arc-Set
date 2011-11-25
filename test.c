#include "tournament.h"
#include "fas_tournament.h"
#include "permutations.h"
#include <time.h>
#include <unistd.h>


void run_tests(char *name, tournament *t){
	printf("%s\n", name);

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

	printf("  Best score from %d samples: %f\n", num_trials, best);
	fas_tournament *ft = run_fas_tournament(t);

	printf("  Optimized Score: %f\n", ft->score);
	
	printf("  Improvement over random sampling: %f%%\n", (100 * (ft->score - best) / best));

	printf("\n\n");

  del_tournament(t);
	del_fas_tournament(ft);
}

int main(){
  srand(time(NULL) ^ getpid());

	size_t n = 1000;

	run_tests("Purely random", random_tournament(n));
	run_tests("Random from voting", random_tournament_from_voting(n));

  return 0;
}
