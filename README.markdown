# Feedback Arc Set solver

This is a library + CLI for solving the following problem:

Given a non-negative nxn matrix W, find a permutation p of [0, n) that maximizes

  sum_{p_i < p_j} W_ij

i.e. we regard W_ij as "the weight of evidence that i < j"

I would only give a fairly qualified recommendation in favour of it. 

Features:

* It does a pretty good job. The test suite requires that it gets within 5% of the best known result on a variety of data, and it beats that with a comfortable margin.
* It's quite fast. For small numbers of items (<= 100) it completes in a few 100ms, going up to a few seconds for thousands of items.
* It's deterministic. Most of the best-of-breed algorithms for this problem are randomized. Empirically, this seems to produce consistently better scores than they do (but that may be errors in my implementation of them)
* It respects condorcet partitions. That is, if you partition the candidates into two sets A and B such that W_ab > W_ba for any a in A and b in B, it will always put everything in A first
* The result is locally optimal in the sense that no change which involves only moving a single element will improve the score
* Everything runs clean under valgrind with all test data
  
Downsides:

* The theoretical bounds on how bad the error can be are extremely weak to non-existent
* The performance is O(n^2) in the number of items, even when far fewer than O(n^2) comparisons are present. 
* The API for the library is fairly poorly thought out at present.
* The command line interface is terribly rudimentary

All that being said, it appears to work and work reasonably well.

Note that the test suite is currently failing. This shouldn't be taken to be a bug - the quality and performance numbers in the set suite are more aspirational than currently achieved. 
