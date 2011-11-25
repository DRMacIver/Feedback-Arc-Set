#include "tournament.h"
#include "fas_tournament.h"
#include "permutations.h"
#include <time.h>
#include <unistd.h>

int main(){
  srand(time(NULL) ^ getpid());

	size_t n = 100;

  tournament *t = random_tournament(n);

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

	printf("With %d trials:\n", num_trials);
	printf("Best score: %f\n", best);
	printf("Average score: %f\n", total / num_trials);

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
