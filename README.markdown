# Feedback Arc Set solver

This is a library + CLI for solving the following problem:

Given a non-negative nxn matrix W, find a permutation p of [0, n) that maximizes

  sum_{p_i < p_j} W_ij

i.e. we regard W_ij as "the weight of evidence that i < j"

I would only give a fairly qualified recommendation in favour of it. 

Features:

* It does a pretty good job. The results are within 1% of the best known ordering I have for any of my test data
* It's quite fast. For small numbers of items (<= 100) it completes in a few 100ms. 
* It's deterministic. There are theoretically better randomized algorithms for this but I wanted it to produce reproducable results.
* It respects condorcet partitions. That is, if you partition the candidates into two sets A and B such that W_ab > W_ba for any a in A and b in B, it will always put everything in A first
* The result is locally optimal int he sense that no change which involves only moving a single element will improve the score
* Everything runs clean under valgrind with all test data
  
Downsides:

* There are no good bounds on how bad the error can be. Even that within 1% claim might be wildly wrong - I don't have optimal orders for many test cases, only best I've been able to find so far.
* The performance is O(n^2) in the number of items. 
* The API for the library is extremely poorly thought out at present.
* The command line interface is terribly rudimentary

All that being said, it appears to work and work reasonably well.
