#include "population.h"
#include <string.h>

population *population_new(size_t population_count, size_t members_size){
  size_t size = sizeof(population_member) * population_count + sizeof(population);
  population *p = malloc(size);
  memset(p, '\0', size);
  p->members_size = members_size;
  p->population_count = population_count;
  return p;
}

void population_del(population *p){
  for(size_t i = 0; i < p->population_count; i++) free(p->members[i].data);
  free(p);
}

static void population_swap(population_member *ms, size_t i, size_t j){
  population_member x = ms[i];
  ms[i] = ms[j];
  ms[j] = x;
}

static void bubble_down(population *p, size_t i){
  if(i >= p->population_count) return;

  size_t c1 = 2 * i + 1;
  size_t c2 = 2 * i + 2;


  if(c1 >= p->population_count) return;

  population_member *cs = p->members;
  size_t smallest_child = (c2 >= p->population_count) ? c1 : (cs[c1].score <= cs[c2].score ? c1 : c2);
  if(cs[smallest_child].score < cs[i].score){
    population_swap(cs, i, smallest_child);
    bubble_down(p, smallest_child);
  }
}

static void heapify_from(population *p, size_t i){
  if(i >= p->population_count) return;
  heapify_from(p, 2 * i + 1);
  heapify_from(p, 2 * i + 2);
  bubble_down(p, i);
}

void population_heapify(population *p){
  heapify_from(p, 0);
}

int population_contains_under(population *p, double key, size_t *data, size_t i){
  if(i >= p->population_count) return 0;
  if(key < p->members[i].score) return 0;
  if(memcmp(data, p->members[i].data, p->members_size * sizeof(size_t))) return 1;
  return population_contains_under(p, key, data, 2*i + 1) || population_contains_under(p, key, data, 2*i + 2);
} 

int population_contains(population *p, double key, size_t *data){
  return population_contains_under(p, key, data, 0);
}

population_member fittest_member(population *p){
  population_member *best_member = p->members;

  for(size_t i = 1; i < p->population_count; i++){
    if(p->members[i].score > best_member->score){
      best_member = p->members + i;
    }
  }

  return *best_member;
}

void population_push(population *p, double key, size_t *data){
  p->members[0].score = key;
  memcpy(p->members[0].data, data, p->members_size* sizeof(size_t));
  bubble_down(p, 0);
}
