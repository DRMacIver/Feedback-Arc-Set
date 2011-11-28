#include <stdlib.h>
#include "tournament.h"

typedef struct {
	double score;
	size_t results;
  size_t *optimal_ordering;
  size_t num_orphans;
  size_t *orphans;
} fas_tournament;

void del_fas_tournament(fas_tournament *t);

fas_tournament *run_fas_tournament(tournament *t);

double score_fas_tournament(tournament *t, size_t count, size_t *data);

size_t tie_starting_from(tournament *t, size_t n, size_t *items, size_t start_index);
