#include <stdlib.h>
#include "tournament.h"

typedef struct {
  int debug;
} fas_tournament_options;

fas_tournament_options default_options();

double score_fas_tournament(tournament *t, size_t count, size_t *data);
size_t *optimal_ordering(fas_tournament_options *options, tournament *t);

size_t tie_starting_from(tournament *t, size_t n, size_t *items, size_t start_index);
size_t condorcet_boundary_from(tournament *t, size_t n, size_t *items, size_t start_index);
