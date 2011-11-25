#include "tournament.h"
#include "permutations.h"

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

double tournament_add(tournament *t, size_t i, size_t j, double x){
  size_t n = t->size;
  return (t->entries[n * i + j] += x);
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

void add_permutation(tournament *t, double weight, size_t *perm){
  size_t n = t->size;

  for(size_t i = 0; i < n; i++){
    for(size_t j = i+1; j < n; j++){
      tournament_add(t, perm[i], perm[j], weight);
    }
  }
}

tournament *random_tournament_from_voting(size_t n){
  tournament *t = new_tournament(n);

  size_t *data = malloc(sizeof(size_t) * n);
  for(size_t i = 0; i < n; i++) data[i] = i;

  size_t sample_count = 10;

  for(int i = 0; i < sample_count; i++){
    shuffle(n, data);
    add_permutation(t, ((double)rand()) / RAND_MAX, data);
  }
  
  free(data); 

  return t;
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
