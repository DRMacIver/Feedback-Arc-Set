import random
import math

def score_fas_tournament(weights, elems):
  score = 0.0

  for i in xrange(len(elems)):
    for j in xrange(i + 1, len(elems)):
      score += weights.get(elems[i], elems[j])

  return score

def optimise(weights, elems = None):
  if not elems:
    elems = xrange(weights.size)

  elems = list(elems)
  best_so_far = list(elems)
  best_score = score_fas_tournament(weights, elems)

  fail_count = 0

  for i in xrange(10000):
    random.shuffle(elems)
    new_score = score_fas_tournament(weights, elems)
    if new_score > best_score:
      fail_count = 0
      best_so_far = list(elems)
      best_score = new_score
    else:
      fail_count += 1
      if fail_count > 100: break

  return best_so_far, best_score
    
def lower_bound(weights):
  tot = 0.0;
  vtot = 0.0;

  for i in xrange(weights.size):
    for j in xrange(i + 1, weights.size):
      aij = weights.get(i, j)
      aji = weights.get(j, i)

      tot += aij
      tot += aji

      vtot += (aij - aji) * (aij - aji);

  return 0.5 * tot + 0.5 * math.sqrt(vtot);


