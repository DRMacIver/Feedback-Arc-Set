import random
import math
from functools import total_ordering

# Public API

def score(weights, elems):
  score = 0.0

  for i in xrange(len(elems)):
    for j in xrange(i + 1, len(elems)):
      score += weights.get(elems[i], elems[j])

  return score

def optimise(weights, elems = None):
  if not elems:
    elems = xrange(weights.size)
  return ranked_pairs(weights, elems)
    
def lower_bound(weights, elems = None):
  if not elems: elems = range(weights.size)

  tot = 0.0;
  vtot = 0.0;

  for i, j in pairs(elems):
    aij = weights.get(i, j)
    aji = weights.get(j, i)

    tot += aij
    tot += aji

    vtot += (aij - aji) ** 2

  return 0.5 * tot + 0.5 * math.sqrt(vtot);

# Module private stuff

def ranked_pairs(weights, elems):
  edges = []

  for x, y in pairs(elems):
    wxy = weights.get(x, y)
    wyx = weights.get(y, x)

    if wxy > wyx: edges.append((x, y, wxy - wyx))
    if wxy < wyx: edges.append((y, x, wyx - wxy))

  edges.sort(key = lambda p: -p[2])

  lesser = {}
  greater = {}

  for x in elems: 
    lesser[x] = set()
    greater[x] = set()

  def add_to_transitive_graph(x, y):
    if x == y: return
    if y in lesser[x] or x in lesser[y]: return

    lesser[y].add(x)
    greater[x].add(y)

    for z in greater[y]: add_to_transitive_graph(x, z)
    for z in lesser[x]: add_to_transitive_graph(z, y)

  for x, y, _ in edges:
    add_to_transitive_graph(x, y)

  @total_ordering
  class GraphKey(object):
    def __init__(self, value):
      self.value = value
    def __gt__(self, other):
      return other.value in lesser[self.value]
    def __eq__(self, other):
      return self.value == other.value
  
  return sorted(elems, key = GraphKey)

def pairs(elems):
  for i in xrange(len(elems)):
    for j in xrange(i + 1, len(elems)):
      yield elems[i], elems[j]
