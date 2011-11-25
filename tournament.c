#include "tournament.h"
#include <string.h>

tournament *new_tournament(int n){
  size_t size = sizeof(tournament) + sizeof(double) * n * n;
  tournament *t = malloc(size);
  memset(t, '\0', size);
  t->size = n;
  return t;
}

void del_tournament(tournament *t){
  free(t);
}

inline double tournament_get(tournament *t, size_t i, size_t j){
  size_t n = t->size;
  return t->entries[n * i + j];
}

double tournament_set(tournament *t, size_t i, size_t j, double x){
  size_t n = t->size;
  return (t->entries[n * i + j] = x);
}

void print_tourmanent(FILE *f, tournament *t){
  size_t n = t-> size;

  for(int i = 0; i < n; i++){
    for(int j = 0; j < n; j++){
      if(j > 0) fprintf(f, " ");
      fprintf(f, "%.2f", tournament_get(t, i, j));
    }
    fprintf(f, "\n");
  }
}

tournament *random_tournament(size_t n){
  tournament *t = new_tournament(n);

  for(int i = 0; i < n; i++){
    for(int j = 0; j < n; j++){
      if(i != j) tournament_set(t, i, j, ((double)rand())/RAND_MAX);
    }
  }
  return t;
}
