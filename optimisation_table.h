#include <stdlib.h>
#include <stdint.h>

typedef struct{
  uint64_t hash;
  size_t length;
  size_t *data;
  double value;
} ot_entry;

typedef struct {
  size_t length;
  size_t occupancy;
  ot_entry *entries;
} optimisation_table;


optimisation_table *optimisation_table_new();
void optimisation_table_del(optimisation_table *ot);
ot_entry *optimisation_table_lookup(optimisation_table *ot, size_t length, size_t *data);
