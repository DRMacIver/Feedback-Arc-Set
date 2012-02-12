#include "fas_tournament.h"
#include "permutations.h"
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>

#define ACCURACY 0.001
#define SMOOTHING 0.05
#define JUST_BRUTE_FORCE_IT 8
#define MAX_MISSES 5
#define MIN_IMPROVEMENT 0.00001

#define FASDEBUG(...) if(_enable_fas_tournament_debug) fprintf(stderr, __VA_ARGS__);

int _enable_fas_tournament_debug = 0;

void enable_fas_tournament_debug(int x){
  _enable_fas_tournament_debug = x;
  FASDEBUG("Debugging on\n");
}

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


typedef struct {
  size_t *buffer;
} fas_optimiser;

static fas_optimiser *new_optimiser(tournament *t){
  fas_optimiser *it = malloc(sizeof(fas_optimiser));
  it->buffer = malloc(sizeof(size_t) * t->size);
  return it;
}

void del_optimiser(fas_optimiser *o){
  free(o);
}

inline double tournament_get(tournament *t, size_t i, size_t j){
  size_t n = t->size;
  assert(i >= 0); 
  assert(i < n); 
  assert(j >= 0); 
  assert(j < n);
  return t->entries[n * i + j];
}

double score_fas_tournament(tournament *t, size_t count, size_t *data){
	double score = 0.0;

	for(size_t i = 0; i < count; i++){
		for(size_t j = i + 1; j < count; j++){
			score +=  t->entries[data[i] * t->size + data[j]];
		}
	}

	return score;
}


static size_t count_tokens(char *c){
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

static int read_line(size_t *buffer_size, char **buffer, FILE *f){
  if(!*buffer) *buffer = malloc(*buffer_size);

  size_t written = 0;
  
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

    (*buffer)[written++] = c;
  }

  if(written == *buffer_size){
    *buffer_size *= 2;
    *buffer = realloc(*buffer, *buffer_size);
  }

  (*buffer)[written] = '\0';

  return 1;
}

static void fail(char *msg){
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

tournament *read_tournament(FILE *f){
  size_t length = 1024;
  char *line = NULL;
  tournament *t;

  if(!read_line(&length, &line, f)){
    fail("No data for read_tournament");
  }

  size_t n = count_tokens(line);

  if(n != 1) fail("Wrong number of entries in header row)");

  char *rest = line;

  n = strtoul(line, &rest, 0);

  if(line == rest){
    fail("I didn't understand the starting line");
  } else if (n <= 0){
    fail("Empty tournament");
  }

  t = new_tournament(n);

  while(read_line(&length, &line, f)){
    char *check = line;
    size_t i = strtoul(line, &rest, 0);
    if(rest == check) fail("failed to parse line"); 
    check = rest;
    size_t j = strtoul(rest, &rest, 0);
    if(rest == check) fail("failed to parse line"); 
    check = rest;
    double f = strtod(rest, &rest);
    if(rest == check) fail("failed to parse line"); 

    if(i >= n || j >= n) fail("index out of bounds");

    t->entries[n * i + j] += f;
  }
  free(line);
  return t;
}

static int tournament_compare(tournament *t, size_t i, size_t j){
	double x = tournament_get(t, i, j);
	double y = tournament_get(t, j, i);

  if(x < y + ACCURACY && x > y - ACCURACY) return 0;

	if(x >= y) return -1;
	if(y >= x) return +1;
	return 0;
}

static inline void swap(size_t *x, size_t *y){
	if(x == y) return;
	size_t z = *x;
	*x = *y;
	*y = z;
}

static void sort(size_t n, size_t *values){
  for(size_t i = 1; i < n; i++){
    size_t k = i;
    while(k > 0 && values[k] < values[k - 1]){
      swap(values + k, values + k - 1);
      k--;
    }
  }
}

static int brute_force_optimise(fas_optimiser *o, tournament *t, size_t n, size_t *items){
	if(n <= 1) return 0;
	if(n == 2){
		int c = tournament_compare(t, items[0], items[1]);
		if(c > 0) swap(items, items+1);
		return c > 0;
	}

	double best_score = score_fas_tournament(t, n, items);

  int changed = 0;

	size_t *working_buffer = o->buffer;
	memcpy(working_buffer, items, n * sizeof(size_t));
  sort(n, working_buffer);

	while(next_permutation(n, working_buffer) < n){
		double score = score_fas_tournament(t, n, working_buffer);

		if(score > best_score){
      changed = 1;
			best_score = score;
			memcpy(items, working_buffer, n * sizeof(size_t));
		}
  }

	return changed;
}

static int window_optimise(fas_optimiser *o, tournament *t, size_t n, size_t *items, size_t window){
  if(n <= window){
    return brute_force_optimise(o, t, n, items);
  }
  double last_score = score_fas_tournament(t, n, items);
  int changed_at_all = 0;
  int changed = 1;
  while(changed){
    changed = 0;
    for(size_t i = 0; i < n - window; i++){
      changed |= brute_force_optimise(o, t, window, items + i); 
    }
    double new_score = score_fas_tournament(t, n, items);

    double improvement = (new_score - last_score) / last_score;
    
    changed_at_all |= changed; 
    if(improvement < MIN_IMPROVEMENT) break;
    last_score = new_score;
  }

  return changed_at_all;
}


typedef struct {
  size_t index;
  double score;
} index_with_score;

int compare_index_with_score(const void *xx, const void *yy){
  index_with_score *x = (index_with_score*)xx;
  index_with_score *y = (index_with_score*)yy;

  if(x->score < y->score) return -1;
  if(x->score > y->score) return 1;
  return 0;
}

// Insertion sort for now. Everything else is O(n^2) anyway
static void sort_by_score(size_t n, double *scores, size_t *values){
  index_with_score *buffer = malloc(n * sizeof(index_with_score));

  for(size_t i = 0; i < n; i++){
    index_with_score *ix = buffer + i;
    ix->index = values[i];
    ix->score = scores[i];
  }

  qsort(buffer, n, sizeof(index_with_score), compare_index_with_score);

  for(size_t i = 0; i < n; i++){
    values[i] = buffer[i].index; 
  }

  free(buffer);
}

static void move_pointer_right(size_t *x, size_t offset){
  while(offset){
    size_t *next = x + 1;
    swap(x, next);
    x = next;
    offset--;
  }
}

static void move_pointer_left(size_t *x, size_t offset){
  while(offset){
    size_t *next = x - 1;
    swap(x, next);
    x = next;
    offset--;
  }
}

static int single_move_optimization(tournament *t, size_t n, size_t *items){
  int changed = 1;
  int changed_at_all = 0;
  while(changed){
    changed = 0;
    for(size_t index_of_interest = 0; index_of_interest < n; index_of_interest++){
      double score_delta = 0;

      if(index_of_interest > 0){
        size_t j = index_of_interest;
        do {
          j--;
          score_delta += tournament_get(t, items[index_of_interest], items[j]);
          score_delta -= tournament_get(t, items[j], items[index_of_interest]);

          if(score_delta > 0){
            move_pointer_left(items+index_of_interest, index_of_interest - j);
            changed = 1; 
            break;
          }
        } while(j > 0);
      }

      for(size_t j = index_of_interest + 1; j < n; j++){
        score_delta += tournament_get(t, items[j], items[index_of_interest]);
        score_delta -= tournament_get(t, items[index_of_interest], items[j]);

        if(score_delta > 0){
          move_pointer_right(items+index_of_interest, j - index_of_interest);
          changed = 1; 
          changed_at_all = 1;
          break;
        }
      }
    }
  }
  return changed_at_all;
}

#define SCORE_SMOOTHING 0.1
static double *initial_scores(tournament *t){
  double *scores = malloc(sizeof(double) * t->size);
  double *working_buffer = malloc(sizeof(double) * t->size);

  for(size_t i = 0; i < t->size; i++){
    scores[i] = 1;
  }

  for(int times = 0; times < 5; times++){
    for(size_t i = 0; i < t->size; i++){
      working_buffer[i] = 0;
    }

    for(size_t i = 0; i < t->size; i++){
      double total_score = t->size * SCORE_SMOOTHING;
      for(size_t j = 0; j < t->size; j++){
        total_score += tournament_get(t, i, j);
      }

      for(size_t j = 0; j < t->size; j++){
        working_buffer[j] += scores[i] * (SCORE_SMOOTHING + tournament_get(t, i, j)) / total_score;
      }
    }

    memcpy(scores, working_buffer, sizeof(double) * t->size);
    double tot = 0.0;
    for(size_t i = 0; i < t->size; i++){
      tot += scores[i];
    }
  }

  free(working_buffer);
  return scores;
}

size_t *integer_range(size_t n){
  size_t *results = malloc(sizeof(size_t) * n);
	for(size_t i = 0; i < n; i++){
		results[i] = i;
	}
  return results;
}

void force_connectivity(tournament *t, size_t n, size_t *items){
  if(!n) return;
  for(size_t i = 0; i < n - 1; i++){
    size_t j = i + 1;
    while(j < n && !tournament_compare(t, items[i], items[j])) j++;
    if(j < n) move_pointer_left(items + j, (j - i - 1));
  }
}


int local_sort(tournament *t, size_t n, size_t *items){
  int changed = 0;
  for(size_t i = 1; i < n; i++){
    size_t j = i;

    while(j > 0 && tournament_compare(t, items[j], items[j - 1]) <= 0){
      changed = 1;
      swap(items + j, items + j - 1);
      j--;
    }
  }
  
  return changed;
}

size_t *optimal_ordering(tournament *t){
  fas_optimiser *o = new_optimiser(t);
  size_t n = t->size;
	size_t *results = integer_range(n);
  FASDEBUG("Scoring\n");
  double *scores = initial_scores(t);
  FASDEBUG("Sorting\n");
  sort_by_score(n, scores, results);
  free(scores);
  force_connectivity(t, n, results);
  local_sort(t, n, results);
  FASDEBUG("  single_move_optimization\n");
  single_move_optimization(t, n, results);

  int smoothing_changed = 0;

  for(int i = 0; i < 5; i++){
    FASDEBUG("Smoothing stage %d\n", i + 1);
    int changed = 0;

    FASDEBUG("  window_optimise(5)\n");
    changed |= window_optimise(o, t, n, results, 5);
    FASDEBUG("  window_optimise(7)\n");
    changed |= window_optimise(o, t, n, results, 7); 
    FASDEBUG("  local_sort\n");
    changed |= local_sort(t, n, results);
    smoothing_changed |= changed;
    if(!changed) break;
  }

  if(smoothing_changed){
    FASDEBUG("  single_move_optimization\n");
    single_move_optimization(t, n, results);
  }

  del_optimiser(o);
  return results;
}

size_t tie_starting_from(tournament *t, size_t n, size_t *items, size_t start_index){
  for(size_t i = start_index+1; i < n; i++){
    for(size_t j = start_index; j < i; j++){
      int c = tournament_compare(t, items[i], items[j]);

      if(c) return i;
    }
  }

  return n;
}

size_t condorcet_boundary_from(tournament *t, size_t n, size_t *items, size_t start_index){
  size_t boundary = start_index;

  int boundary_change = 0;
  do {
    boundary_change = 0;

    for(size_t i = start_index; i <= boundary; i++){
      for(size_t j = boundary + 1; j < n; j++){
        if(tournament_compare(t, items[j], items[i]) <= 0){
          boundary = j;
          boundary_change = 1;
          break; 
        }
      }     
    }
  } while(boundary_change);

  return boundary;
}

