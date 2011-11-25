#include "fas_tournament.h"
#include "permutations.h"
#include <string.h>

void del_fas_tournament(fas_tournament *t){
  free(t->optimal_ordering);
  free(t->orphans);
  free(t);
}

double score_fas_tournament(tournament *t, size_t count, size_t *data){
	double score = 0.0;

	for(size_t i = 0; i < count; i++){
		for(size_t j = i + 1; j < count; j++){
			score += tournament_get(t, data[i], data[j]);
		}
	}

	return score;
}

fas_tournament *run_fas_tournament(tournament *t){
	if(t->size == 0) return NULL;

	fas_tournament *ft = malloc(sizeof(fas_tournament));

	ft->num_orphans = 0;
	ft->orphans = NULL;

	size_t n = t->size;

	ft->results = n;

	double best_score = -1.0;

	size_t *results = malloc(sizeof(size_t) * n);
	ft->optimal_ordering = results;

	size_t *working_buffer = malloc(sizeof(size_t) * n);

	for(size_t i = 0; i < n; i++){
		working_buffer[i] = i;
	}

	while(next_permutation(n, working_buffer) < n){
		double score = score_fas_tournament(t, n, working_buffer);

		if(score > best_score){
			best_score = score;
			memcpy(results, working_buffer, n * sizeof(size_t));
		}
  }

	ft->score = best_score;

	free(working_buffer);

	return ft;
}
