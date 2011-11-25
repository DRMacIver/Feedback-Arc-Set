#include "tournament.h"
#include "permutations.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

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

void print_tournament(FILE *f, tournament *t){
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

size_t count_tokens(char *c){
  if(*c == '\0') return 0;

  size_t count = 0;
  int in_token = !isspace(*c);
  
  while(*c){
    if(isspace(*c)){
      if(in_token) count++;
      in_token = 0;
    } else {
      in_token = 1;
    }
    c++;
  }

  if(in_token) count++;

  return count;
}

int read_line(size_t *buffer_size, char **buffer, FILE *f){
  if(!*buffer) *buffer = malloc(*buffer_size);

  size_t written = 0;
  
  char *target = *buffer;
 
  for(;;){
    char c = getc(f);

    if(c == EOF){
      if(written) break;
      else return 0;
    }
    if(c == '\n') break;

    if(written == *buffer_size){
      *buffer_size *= 2;
      *buffer = realloc(*buffer, *buffer_size);
    }

    target[written++] = c;
  }

  if(written == *buffer_size){
    *buffer_size *= 2;
    *buffer = realloc(*buffer, *buffer_size);
  }

  target[written] = '\0';

  return 1;
}

void fail(char *msg){
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

tournament *read_tournament(FILE *f){
  size_t length = 1024;
  char *line = NULL;

  if(!read_line(&length, &line, f)){
    fail("No data for read_tournament");
  }

  size_t n = count_tokens(line);

  if(n == 0) fail("Empty line in read_tournament)");

  tournament *t = new_tournament(n);

  size_t i = 0;
  do {
    size_t j = 0;
    
    char *start = line;
    char *rest = line;

    while(*start){
      if(j >= n) fail("Too many entries");

      double f = strtod(start, &rest);

      if(rest == start) fail("Failed to read line");

      tournament_set(t, i, j, f);

      j++;
      start = rest;
    }
     
    i++;
  } while(i < n && read_line(&length, &line, f));

  return t;
}
