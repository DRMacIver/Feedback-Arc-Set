#include <stdlib.h>
#include "tournament.h"

double score_fas_tournament(tournament *t, size_t count, size_t *data);
size_t *optimal_ordering(tournament *t);

size_t tie_starting_from(tournament *t, size_t n, size_t *items, size_t start_index);
size_t condorcet_boundary_from(tournament *t, size_t n, size_t *items, size_t start_index);
