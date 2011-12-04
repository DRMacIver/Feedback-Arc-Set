#ifndef TOURNAMENT_H_INCLUDED
#define TOURNAMENT_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

typedef struct {
  size_t size;
  double entries[];
} tournament;

tournament *new_tournament(int n);
void del_tournament(tournament *t);
double tournament_get(tournament *t, size_t i, size_t j);
double tournament_set(tournament *t, size_t i, size_t j, double x);
double tournament_add(tournament *t, size_t i, size_t j, double x);
void print_tournament(FILE *f, tournament *t);
tournament *read_tournament(FILE *f);
tournament *random_tournament(size_t n);
tournament *random_tournament_from_voting(size_t n);

#endif
