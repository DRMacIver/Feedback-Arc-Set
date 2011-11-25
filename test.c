#include "tournament.h"

int main(){
  tournament *t = random_tournament(20);

  print_tourmanent(stdout, t);

  del_tournament(t);

  return 0;
}
