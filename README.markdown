# Feedback Arc Set solver

This is a library + CLI for solving the following problem:

Given a non-negative nxn matrix W, find a permutation p of [0, n) that maximizes

  sum_{p_i < p_j} W\_ij

i.e. we regard W\_ij as "the weight of evidence that i < j"

I would only give a fairly qualified recommendation in favour of it. 

Features:

* It does a pretty good job. The results are within 2% of the best known ordering I have for any of my test data (the goal is within 1% - not currently achieved for some data)
* It's moderately fast. For small numbers of items, say under 50, it completes in a few 100ms. 
* It's deterministic by default. There are theoretically better randomized algorithms for this but I wanted it to produce reproducable results.
* It respects condorcet partitions. That is, if you partition the candidates into two sets A and B such that W_ab > W_ba for any a in A and b in B, it will always put everything in A first
* The result is locally optimal in the sense that no change which involves only moving a single element will improve the score (and a whole bunch of other senses)
* Everything runs clean under valgrind with all test data
  
Downsides:

* There are no good bounds on how bad the error can be. Even that within 1% claim might be wildly wrong - I don't have optimal orders for many test cases, only best I've been able to find so far.
* The performance is O(n^2) in the number of items. 
* The API for the library is extremely poorly thought out at present.
* The command line interface is terribly rudimentary

All that being said, it appears to work and work reasonably well.

Note that the test suite is currently failing. This shouldn't be taken to be a bug - the quality and performance numbers in the set suite are more aspirational than currently achieved. 

Notes:

* An astute observer will see that the test suite is self modifying. Whenever the code manages to find a better ordering it modifies the test data to include that. 
* At this stage I'm pretty confident that the orderings in there are near optimal. They've been subjected to a whole variety of different algorithms and runs to produce them and it's more or less stopped improving at this stage. Even on the rare occasions where an ordering is improved it tends to be by a few 1/00ths of a percent.
* There are some command line flags to make the system try harder by introducing non-determinism. Use them at your risk. 
