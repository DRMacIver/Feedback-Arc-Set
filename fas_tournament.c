#include "fas_tournament.h"
#include "permutations.h"
#include <string.h>
#include <assert.h>
#include <math.h>

#define ACCURACY 0.001
#define SMOOTHING 0.05
#define JUST_BRUTE_FORCE_IT 8
#define MAX_MISSES 5
#define MIN_IMPROVEMENT 0.00001

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
		for(size_t j = i + 1; j < count; j++){
			score += tournament_get(t, data[i], data[j]);
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

int brute_force_optimise(tournament *t, size_t n, size_t *items){
	if(n <= 1) return 0;
	if(n == 2){
		int c = tournament_compare(t, items[0], items[1]);
		if(c > 0) swap(items, items+1);
		return c > 0;
	}

	double best_score = score_fas_tournament(t, n, items);

  int changed = 0;

	size_t *working_buffer = malloc(sizeof(size_t) * n);
	memcpy(working_buffer, items, n * sizeof(size_t));

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

    double *new_scores = malloc(sizeof(double) * t->size);

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
  double *scores = malloc(sizeof(double) * t->size);
  double *working_buffer = malloc(sizeof(double) * t->size);

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
  size_t *results = malloc(sizeof(size_t) * n);
	for(size_t i = 0; i < n; i++){
		results[i] = i;
	}
  return results;
}

void heavy_duty_smoothing(fas_tournament_options *opts, tournament *t, size_t n, size_t *items){
  if(opts->debug) fprintf(stderr, "Optimising subranges\n");
  optimise_subranges_thoroughly(t, n, items);
  if(opts->debug) fprintf(stderr, "Done. Score is now %f\n", score_fas_tournament(t, n, items));
  if(opts->debug) fprintf(stderr, "Window optimising with a window of 5\n");
  while(window_optimise(t, n, items, 5) || single_move_optimization(t, n, items)); 
  if(opts->debug) fprintf(stderr, "Done. Score is now %f\n", score_fas_tournament(t, n, items));
  if(opts->debug) fprintf(stderr, "Window optimising with a window of 8\n");
  window_optimise(t, n, items, 8); 
  single_move_optimization(t, n, items);
  if(opts->debug) fprintf(stderr, "Done. Score is now %f\n", score_fas_tournament(t, n, items));
  if(opts->debug) fprintf(stderr, "Cycling subranges\n");
  while(cycle_all_subranges(t, n, items, 25) || single_move_optimization(t, n, items));
  if(opts->debug) fprintf(stderr, "Done. Score is now %f\n", score_fas_tournament(t, n, items));
}

void copy_range(size_t *it, size_t n, size_t *items){
  memcpy(it, items, n * sizeof(size_t));
}

size_t *clone_range(size_t n, size_t *items){
  size_t *it = malloc(n * sizeof(size_t));
  copy_range(it, n, items);
  return it;
}

int with_probability(double p){
  return rand() < p * RAND_MAX;
}

void anneal(fas_tournament_options *opts, tournament *t, size_t n, size_t *items){
  assert(n >= 2);
  double best_score = score_fas_tournament(t, n, items);
  double worst_score = best_score;

  size_t *current_state  = clone_range(n, items);

  size_t failure_count = 0;

  for(size_t i = 0; i < 10000; i++){
    shuffle(n, current_state);
    double score = score_fas_tournament(t, n, current_state);

    if(score < worst_score) worst_score = score;
    if(score > best_score){
      if(opts->debug) fprintf(stderr, "Shuffling improved best score %f -> %f after %lu iterations\n", best_score, score, i);
      failure_count = 0;
      memcpy(items, current_state, sizeof(size_t) * n);
      best_score = score;
    } else {
      if(failure_count >= 100){
        if(opts->debug) fprintf(stderr, "Stopping shuffling after %lu iterations with no improvement\n", failure_count);
        break;
      }
      failure_count++;
    }
  }

  if(opts->debug) fprintf(stderr, "Scores lie in range [%f, %f]\n", worst_score, best_score);

  double starting_score = best_score;

  double temperature = best_score;
  double cooling_rate = 0.95;
  double zero_point = 0.001;
  double restart_probability = 0.05;
  size_t max_restarts = 100;
  size_t current_restarts = 0;
  double current_score = best_score;

  while(temperature > zero_point){
    size_t i = random_number(n);
    size_t j = random_number(n);
  
    if(i == j) continue;
    if(i > j) swap(&i, &j);
  
    reverse(current_state + i, current_state + j);

    double new_score = score_fas_tournament(t, n, current_state);

    double p = exp((new_score - current_score) / temperature);

    if(new_score > best_score){
      if(opts->debug) fprintf(stderr, "Simulated annealing improved best score %f -> %f\n", best_score, new_score);
      best_score = new_score;
      copy_range(items, n, current_state);
      current_restarts = 0;
    }

    if((new_score > current_score) || with_probability(p)){
      current_score = new_score;
    } else {
      if(with_probability(restart_probability)){
        if(current_restarts >= max_restarts) break;
        current_restarts++;
        current_score = best_score;
        copy_range(current_state, n, items); 
      } else {
        reverse(current_state + i, current_state + j);
      }
    }
    temperature *= cooling_rate;
  }
  if(opts->debug) fprintf(stderr, "Simulated annealing finished with best score %f (%.2f%% improvement)\n", best_score, ((best_score - starting_score) / starting_score) * 100);

  free(current_state);
}

size_t *optimal_ordering(fas_tournament_options *options, tournament *t){
  size_t n = t->size;
	size_t *results = integer_range(n);
  if(options->debug) fprintf(stderr, "Starting score %f\n", score_fas_tournament(t, n, results));

  anneal(options, t, n, results);
  heavy_duty_smoothing(options, t, n, results);

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

fas_tournament_options default_options(){
  fas_tournament_options result;

  result.debug = 0;

  return result;
}
