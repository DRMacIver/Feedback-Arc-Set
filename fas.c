#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "tournament.h"
#include "fas_tournament.h"

int has_arg(char *flag, int argc, char **argv){
  for(int i = 0; i < argc; i++){
    if(!strcmp(flag, argv[i])) return 1;
  } 
  return 0;
}

char *remove_prefix(char *prefix, char *str){
  while(*prefix && *str){
    if(*prefix != *str) return NULL;
    prefix++;
    str++;
  }
  if(*prefix && !*str) return NULL;
  return str;
}

char *get_arg(char *flag, int argc, char **argv){
  for(int i = 0; i < argc; i++){
    char *result = remove_prefix(flag, argv[i]);
    if(result) return result;
  } 
  return NULL;
}

int main(int argc, char **argv){
  srand(time(NULL) ^ getpid());
  FILE *argf = stdin;

  fas_tournament_options options = default_options();

  if(has_arg("--shuffle", argc, argv)){
    options.include_shuffle_pass = 1;
  }

  if(has_arg("--anneal", argc, argv)){
    options.include_annealing_pass = 1;
  }

  char *infile = get_arg("--infile=", argc, argv);

  if(infile){
    argf = fopen(infile, "r");

    if(!argf){
      fprintf(stderr, "Unable to open file %s for reading\n", infile);
      exit(1);
    } 
  }

	tournament *t = read_tournament(argf);

  size_t n = t->size; 
	size_t *items = optimal_ordering(&options, t);

	printf("Score: %f\n", score_fas_tournament(t, n, items));
	printf("Optimal ordering:");

  size_t i = 0;
  size_t next_boundary = condorcet_boundary_from(t, n, items, i);

  for(;;){
    size_t next_i = tie_starting_from(t, n, items, i);

    if(next_i > i + 1){
      printf(" [");
      for(size_t j = i; j < next_i; j++){
        if(j > i) printf(" ");
        printf("%lu", items[j]);
      }
      printf("]");
    } else {
      printf(" %lu", items[i]);
    }

    if(next_i == n) break;

    i = next_i;
    if(i > next_boundary){
      printf(" ||");
      next_boundary = condorcet_boundary_from(t, n, items, i);
    }
	}
  printf("\n");

  free(items);
	del_tournament(t);

	return 0;
}
