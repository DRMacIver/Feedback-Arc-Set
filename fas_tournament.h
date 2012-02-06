#include <stdlib.h>
#include <stdio.h>

typedef struct {
  size_t row;
  size_t column;
  double value;
} fas_entry;

typedef struct {
  size_t size;
  size_t entry_count;
  fas_entry entries[];
} tournament;

void enable_fas_tournament_debug(int x);

tournament *new_tournament(int n);
void del_tournament(tournament *t);
double tournament_get(tournament *t, size_t i, size_t j);

tournament *read_tournament(FILE *f);

double best_score_lower_bound(tournament *t, size_t n, size_t *items);
double score_fas_tournament(tournament *t, size_t count, size_t *data);
size_t *optimal_ordering(tournament *t);

size_t tie_starting_from(tournament *t, size_t n, size_t *items, size_t start_index);
size_t condorcet_boundary_from(tournament *t, size_t n, size_t *items, size_t start_index);
