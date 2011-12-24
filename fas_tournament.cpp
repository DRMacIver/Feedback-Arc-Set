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

tournament *new_tournament(int n){
  size_t size = sizeof(tournament) + sizeof(double) * n * n;
  tournament *t = (tournament*)malloc(size);
  memset(t, '\0', size);
  t->size = n;
  return t;
}

void del_tournament(tournament *t){
  free(t);
}


#define CHECK_INDICES assert(i >= 0); assert(i < n); assert(j >= 0); assert(j < n);

inline double tournament_get(tournament *t, size_t i, size_t j){
  size_t n = t->size;
  CHECK_INDICES
  return t->entries[n * i + j];
}

double tournament_set(tournament *t, size_t i, size_t j, double x){
  size_t n = t->size;
  CHECK_INDICES
  return (t->entries[n * i + j] = x);
}

double tournament_add(tournament *t, size_t i, size_t j, double x){
  size_t n = t->size;
  CHECK_INDICES
  return (t->entries[n * i + j] += x);
}

void print_tournament(FILE *f, tournament *t){
  size_t n = t-> size;

  for(size_t i = 0; i < n; i++){
    for(size_t j = 0; j < n; j++){
      if(j > 0) fprintf(f, " ");
      fprintf(f, "%.2f", tournament_get(t, i, j));
    }
    fprintf(f, "\n");
  }
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
  if(!*buffer) *buffer = (char*) malloc(*buffer_size);

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
      *buffer = (char*)realloc(*buffer, *buffer_size);
    }

    (*buffer)[written++] = c;
  }

  if(written == *buffer_size){
    *buffer_size *= 2;
    *buffer = (char*)realloc(*buffer, *buffer_size);
  }

  (*buffer)[written] = '\0';

  return 1;
}

void fail(char *msg){
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

  if(n == 0) fail("Empty line in read_tournament)");

  if(n == 1){
    // We interpret this as sparse matrix format

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
    
      tournament_add(t, i, j, f);
    }

  } else {
    t = new_tournament(n);

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
  }

  free(line);
  return t;
}

int tournament_compare(tournament *t, size_t i, size_t j){
	double x = tournament_get(t, i, j);
	double y = tournament_get(t, j, i);

  if(x < y + ACCURACY && x > y - ACCURACY) return 0;

	if(x >= y) return -1;
	if(y >= x) return +1;
	return 0;
}

double score_fas_tournament(tournament *t, size_t count, size_t *data){
	double score = 0.0;

	for(size_t i = 0; i < count; i++){
    double *score_array = t->entries + data[i] * t->size;

		for(size_t j = i + 1; j < count; j++){
			score += score_array[data[j]];
		}
	}

	return score;
}

// IMPORTANT: Returns an INDEX not an ELEMENT
size_t pick_a_pivot(tournament *t, size_t count, size_t *items){
	return count / 2;
}


static inline void swap(size_t *x, size_t *y){
	if(x == y) return;
	size_t z = *x;
	*x = *y;
	*y = z;
}

void sort(size_t n, size_t *values){
  for(size_t i = 1; i < n; i++){
    size_t k = i;
    while(k > 0 && values[k] < values[k - 1]){
      swap(values + k, values + k - 1);
      k--;
    }
  }
}

int brute_force_optimise(tournament *t, size_t n, size_t *items){
	if(n <= 1) return 0;
	if(n == 2){
		int c = tournament_compare(t, items[0], items[1]);
		if(c > 0) swap(items, items+1);
		return c > 0;
	}

	double best_score = score_fas_tournament(t, n, items);

  int changed = 0;

	size_t *working_buffer = (size_t*)malloc(sizeof(size_t) * n);
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

	free(working_buffer);

	return changed;
}

int window_optimise(tournament *t, size_t n, size_t *items, size_t window){
  if(n <= window){
    return brute_force_optimise(t, n, items);
  }
  double last_score = score_fas_tournament(t, n, items);
  int changed_at_all = 0;
  int changed = 1;
  while(changed){
    changed = 0;
    for(size_t i = 0; i < n - window; i++){
      changed |= brute_force_optimise(t, window, items + i); 
    }
    double new_score = score_fas_tournament(t, n, items);

    double improvement = (new_score - last_score) / last_score;
    
    changed_at_all |= changed; 
    if(improvement < MIN_IMPROVEMENT) break;
    last_score = new_score;
  }

  return changed_at_all;
}

int local_sort(tournament *t, size_t n, size_t *items){
  int changed = 0;
  for(size_t i = 1; i < n; i++){
    for(size_t k = i; k > 0; k--){
      int c = tournament_compare(t, items[k-1], items[k]);
      if(c <= 0) break;
      changed = 1;
      swap(items + k - 1, items + k);
    }  
  }
  return changed;
}

int double_compare(double x, double y){
  return (x > y) - (y > x);
}

// Insertion sort for now. Everything else is O(n^2) anyway
void sort_by_score(size_t n, double *scores, size_t *values){
  for(size_t i = 1; i < n; i++){
    size_t k = i;
    while(k > 0 && scores[values[k]] < scores[values[k - 1]]){
      swap(values + k, values + k - 1);
      k--;
    }
  }
}

void multisort_by_score(tournament *t, double *scores, size_t n, size_t *items){
  sort_by_score(n, scores, items);

  if(n <= JUST_BRUTE_FORCE_IT) brute_force_optimise(t, n, items);
  else {
    size_t k = n/2;
    size_t pivot = items[k];

    double *new_scores = (double*)malloc(sizeof(double) * t->size);

    for(size_t i = 0; i < t->size; i++){
      new_scores[i] = tournament_get(t, pivot, i);
    }

    sort_by_score(n, new_scores, items);

    multisort_by_score(t, scores, k, items);
    multisort_by_score(t, scores, n - k, items + k);
  }
}

void move_pointer_right(size_t *x, size_t offset){
  while(offset){
    size_t *next = x + 1;
    swap(x, next);
    x = next;
    offset--;
  }
}

void move_pointer_left(size_t *x, size_t offset){
  while(offset){
    size_t *next = x - 1;
    swap(x, next);
    x = next;
    offset--;
  }
}

int single_move_optimization(tournament *t, size_t n, size_t *items){
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
double *initial_scores(tournament *t){
  double *scores = (double*)malloc(sizeof(double) * t->size);
  double *working_buffer = (double*)malloc(sizeof(double) * t->size);

  for(size_t i = 0; i < t->size; i++){
    scores[i] = 1;
  }

  for(int times = 0; times < 20; times++){
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

void shuffle_optimisation(tournament *t, size_t n, size_t *items){
  size_t *working_buffer = (size_t*)malloc(sizeof(size_t) * n);

  memcpy(working_buffer, items, sizeof(size_t) * n);

  double best_score = score_fas_tournament(t, n, items);

  size_t failure_count = 0;

  for(size_t i = 0; i < 1000; i++){
    shuffle(n, working_buffer);
    window_optimise(t, n, working_buffer, 5);
    single_move_optimization(t, n, working_buffer);
    double score = score_fas_tournament(t, n, working_buffer);

    if(score > best_score){
      failure_count = 0;
      memcpy(items, working_buffer, sizeof(size_t) * n);
      best_score = score;
    } else {
      failure_count++;
      if(failure_count > 50) break;
    }
  }

  free(working_buffer);
}

void optimise_pretty_thoroughly(tournament *t, size_t n, size_t *items){
  if(n <= 10){
    brute_force_optimise(t, n, items);
  } else {
    shuffle_optimisation(t, n, items);
    for(size_t i = 0; i < 1000; i++){
      if(!(window_optimise(t, n, items, 9) || single_move_optimization(t, n, items))) break;
    }

    size_t split = n / 2;
    optimise_pretty_thoroughly(t, split, items);
    optimise_pretty_thoroughly(t, n - split, items + split);
  }
}

#define CHUNK_SIZE 8
void optimise_subranges_thoroughly(tournament *t, size_t n, size_t *items){
  for(size_t i = 0; i < n; i += CHUNK_SIZE){
    size_t length = CHUNK_SIZE;
    if(i + length > n) length = n - i;
    brute_force_optimise(t, length, items + i);
  }
}

void rotate_array(size_t n, size_t *items, size_t k){
  if(!k) return;

  k = k % n;
  for(size_t i = 0; i < k; i++){
    size_t new_start = items[n - 1];
    for(size_t j = n - 1; j > 0; j--){
      items[j] = items[j-1];
    }
    items[0] = new_start;
  }
}

size_t find_best_cycle(tournament *t, size_t n, size_t *items){
  double best_score = score_fas_tournament(t, n, items);
  size_t best_index = 0;

  for(size_t k = 1; k < n; k++){
    rotate_array(n, items, 1);
    double score =  score_fas_tournament(t, n, items);

    if(score > best_score){
      best_score = score;
      best_index = k;
    }
  }
  rotate_array(n, items, 1);
  if(!best_index) return 0;

  rotate_array(n, items, best_index);

  return best_index;
}

int cycle_all_subranges(tournament *t, size_t n, size_t *items, size_t max_length){
  int changed = 0;

  for(size_t length = 2; length < max_length; length++){
    for(size_t start = 0; start + length < n; start++){
      changed |= find_best_cycle(t, length, items+start);
    }
  }
  return changed;
}

size_t *integer_range(size_t n){
  size_t *results = (size_t*)malloc(sizeof(size_t) * n);
	for(size_t i = 0; i < n; i++){
		results[i] = i;
	}
  return results;
}

void heavy_duty_smoothing(tournament *t, size_t n, size_t *items){
  optimise_subranges_thoroughly(t, n, items);
  while(window_optimise(t, n, items, 5) || single_move_optimization(t, n, items)); 
  window_optimise(t, n, items, 8); 
  single_move_optimization(t, n, items);
  while(cycle_all_subranges(t, n, items, 25) || single_move_optimization(t, n, items));
}

double best_score_lower_bound(tournament *t, size_t n, size_t *items){
  double tot = 0.0;
  double vtot = 0.0;

  for(size_t i = 0; i < n; i++){
    for(size_t j = i+1; j < n; j++){
      double aij = tournament_get(t, items[i], items[j]);
      double aji = tournament_get(t, items[j], items[i]);

      tot += aij;
      tot += aji;

      vtot += (aij - aji) * (aij - aji);
    }
  }

  return 0.5 * tot + 0.5 * sqrt(vtot);
}

int shuffle_to_optimality(tournament *t, size_t n, size_t *items){
  if(n <= JUST_BRUTE_FORCE_IT){
    return brute_force_optimise(t, n, items);
  }  

  int changed = 0;
  while(score_fas_tournament(t, n, items) < best_score_lower_bound(t, n, items)){
    changed = 1;
    shuffle(n, items);
  }
  return changed;
}

void shuffle_subranges_to_optimality(tournament *t, size_t n, size_t *items){
  if(n <= JUST_BRUTE_FORCE_IT){
    brute_force_optimise(t, n, items);
    return;
  }  

  size_t max_failures = 100;

  size_t failure_count = 0;

  while(failure_count < max_failures){
    size_t i = random_number(n);
    size_t j = random_number(n);

    if(i == j) continue;
    if(i > j) swap(&i, &j);
    
    if(shuffle_to_optimality(t, j - i, items + i)) failure_count = 0;
    else failure_count++;
  }

}

size_t *optimal_ordering(tournament *t){
  size_t n = t->size;
	size_t *results = integer_range(n);
  double *scores = initial_scores(t);
  shuffle_to_optimality(t, n, results);
  multisort_by_score(t, scores, n, results);
  heavy_duty_smoothing(t, n, results);
  free(scores);
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

