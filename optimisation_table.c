#include "optimisation_table.h"
#include <string.h>
#include <stdio.h>

#define MAX_OCCUPANCY_RATIO 0.9
#define DEFAULT_TABLE_SIZE 65536

inline int set_contains_value(size_t length, size_t *xs, size_t x){
  for(size_t i = 0; i < length; i++) if(xs[i] == x) return 1;
  return 0;
}

int set_compare(size_t length, size_t *x, size_t *y){
  for(size_t i = 0; i < length; i++) if(!set_contains_value(length, y, x[i])) return 0;
  return 1;
}

uint64_t hash64(uint64_t key)
{
  key = (~key) + (key << 21);
  key = key ^ (key >> 24);
  key = (key + (key << 3)) + (key << 8);
  key = key ^ (key >> 14);
  key = (key + (key << 2)) + (key << 4);
  key = key ^ (key >> 28);
  key = key + (key << 31);
  return key;
}

uint64_t set_hash(size_t length, size_t *x){
  uint64_t result = 0;
  for(size_t i = 0; i < length; i++) result ^= hash64(x[i]);
  return result;
}

size_t *clone_set(size_t length, size_t *x){
  size_t *result = malloc(length * sizeof(size_t));
  memcpy(result, x, length * sizeof(size_t));
  return result;
}

optimisation_table *optimisation_table_new(){
  optimisation_table *result = malloc(sizeof(optimisation_table));
  result->occupancy = 0;
  result->length=DEFAULT_TABLE_SIZE;
  result->entries = calloc(DEFAULT_TABLE_SIZE, sizeof(ot_entry));
  return result;
}

void optimisation_table_del(optimisation_table *ot){
  for(size_t i = 0; i < ot->length; i++) free(ot->entries[i].data);
  free(ot->entries); 
  free(ot);
}

ot_entry *optimisation_table_lookup_internal(optimisation_table *ot, size_t length, size_t *data, int must_copy){
  uint64_t h = set_hash(length, data);

  size_t mask = ot->length - 1;
  size_t p = (size_t)(h & mask);

  while(1){
    ot_entry *ce = ot->entries + p;
    if(!ce->length){
      if(ot->occupancy > ot->length * MAX_OCCUPANCY_RATIO){
        size_t old_length = ot->length;
        ot_entry *old_entries = ot->entries;
        ot->length *= 2;
        ot->entries = calloc(ot->length, sizeof(ot_entry));

        for(size_t i = 0; i < old_length; i++){
          if(old_entries[i].length){
            optimisation_table_lookup_internal(ot, old_entries[i].length, old_entries[i].data, 0)->value = old_entries[i].value;
          }
        }
        free(old_entries);
        return optimisation_table_lookup_internal(ot, length, data, must_copy);
      } else {
        ce->length = length;
        ce->hash = h;
        ce->data = (must_copy ? clone_set(length, data) : data);
        ce->value = -1;
        ot->occupancy++;
        return ce;
      }
    } else {
      if((ce->length == length) && (ce->hash == h) && set_compare(length, data, ce->data)) return ce;
    }

    p = (p + 1) & mask;
  }
}

ot_entry *optimisation_table_lookup(optimisation_table *ot, size_t length, size_t *data){
  return optimisation_table_lookup_internal(ot, length, data, 1);
}
