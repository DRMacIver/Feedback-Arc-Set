#include <stdlib.h>
#include <stdio.h>

typedef struct {
  size_t size;
  double entries[1];
} tournament;

tournament *new_tournament(int n);
void del_tournament(tournament *t);
double tournament_get(tournament *t, size_t i, size_t j);
double tournament_set(tournament *t, size_t i, size_t j, double x);
double tournament_add(tournament *t, size_t i, size_t j, double x);

void print_tournament(FILE *f, tournament *t);
tournament *read_tournament(FILE *f);

double best_score_lower_bound(tournament *t, size_t n, size_t *items);
double score_fas_tournament(tournament *t, size_t count, size_t *data);
size_t *optimal_ordering(tournament *t);

size_t tie_starting_from(tournament *t, size_t n, size_t *items, size_t start_index);
size_t condorcet_boundary_from(tournament *t, size_t n, size_t *items, size_t start_index);
