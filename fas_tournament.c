#include "fas_tournament.h"
#include "permutations.h"
#include <string.h>
#include <assert.h>

#define ACCURACY 0.001
#define SMOOTHING 0.05
#define JUST_BRUTE_FORCE_IT 8
#define MAX_MISSES 5
#define MIN_IMPROVEMENT 0.00001

void del_fas_tournament(fas_tournament *t){
  free(t->optimal_ordering);
  free(t->orphans);
  free(t);
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

void kwik_sort(tournament *t, double *scores, size_t n, size_t *items){
  size_t *save_buffer = malloc(n * sizeof(size_t));
  memcpy(save_buffer, items, n * sizeof(size_t));
  double starting_score = score_fas_tournament(t, n, items);

	if(n <= JUST_BRUTE_FORCE_IT){
		brute_force_optimise(t, n, items);
		return;
	}

	size_t i = pick_a_pivot(t, n, items);
	size_t v = items[i];

	swap(items + i, items);

	// Simple three-way partition
	// While sorting we have:
	// [< v | ==v | ??? | > v ]

	size_t *unsorted_start = items + 1;

	size_t *lt_end = items;
	
	// WARNING: Past end of array
	size_t *gt_begin = items + n;

	while(unsorted_start < gt_begin){	
		int c = double_compare(scores[v], scores[*unsorted_start]);

		if(c < 0){
			swap(lt_end, unsorted_start);
			lt_end++;	
			unsorted_start++;
		}
		else if (c == 0){
			unsorted_start++;
		} else {
			gt_begin--;
			swap(unsorted_start, gt_begin);
		}	
	}

	kwik_sort(t, scores, (lt_end - items), items);
	kwik_sort(t, scores, (items + n - gt_begin), gt_begin);

	if((gt_begin - lt_end) < n){
		kwik_sort(t, scores, (gt_begin - lt_end), lt_end);
	}

  if(score_fas_tournament(t, n, items) < starting_score){
    // That didn't go so well
    memcpy(items, save_buffer, n * sizeof(size_t));
  }

  free(save_buffer);
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
      for(size_t j = 0; j < t->size; j++){
        double ij = tournament_get(t, i, j);
        double ji = tournament_get(t, j, i);

        // Smoothed probability that i < j
        double p = (ij + SMOOTHING) / (ij + ji + SMOOTHING * 2);

        working_buffer[j] += scores[i] * p / t -> size;
        working_buffer[i] += scores[i] * (1 - p) / t -> size;
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
  size_t *working_buffer = malloc(sizeof(size_t) * n);

  memcpy(working_buffer, items, sizeof(size_t) * n);

  double best_score = score_fas_tournament(t, n, items);

  size_t failure_count = 0;

  for(size_t i = 0; i < 1000; i++){
    shuffle(n, working_buffer);
    window_optimise(t, n, working_buffer, 5);
    single_move_optimization(t, n, working_buffer);
    double score = score_fas_tournament(t, n, working_buffer);

    if(score > best_score){
      fprintf(stderr, "Score: %f -> %f\n", best_score, score);
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


fas_tournament *run_fas_tournament(tournament *t){
	if(t->size == 0) return NULL;

  double *scores = initial_scores(t);

	fas_tournament *ft = malloc(sizeof(fas_tournament));

	ft->num_orphans = 0;
	ft->orphans = NULL;

	size_t n = t->size;

	ft->results = n;
	size_t *results = malloc(sizeof(size_t) * n);
	for(size_t i = 0; i < n; i++){
		results[i] = i;
	}

	size_t *working_buffer = malloc(sizeof(size_t) * n);
	memcpy(working_buffer, results, n * sizeof(size_t));

  kwik_sort(t, scores, n, working_buffer);
  single_move_optimization(t, n, working_buffer);
  window_optimise(t, n, working_buffer, 5);
  single_move_optimization(t, n, working_buffer);
  memcpy(results, working_buffer, sizeof(size_t) * n);

	ft->optimal_ordering = results;
  ft->score = score_fas_tournament(t, n, results);

  free(working_buffer);
  free(scores);

	return ft;
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
