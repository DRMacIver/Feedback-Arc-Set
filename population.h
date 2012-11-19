#include <stdlib.h>

typedef struct {
  double score;
  size_t *data;
} population_member;

typedef struct {
  size_t members_size;
  size_t population_count;
  population_member members[1];
} population;

population *population_new(size_t population_count, size_t members_size);
void population_del(population *p);

void population_heapify(population *p);
int population_contains(population *p, double key, size_t *data);

size_t *fittest_member(population *p);
