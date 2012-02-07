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

# Input format

The input to the solver is a sparse matrix format. It takes the form

    n
    i1 j1 x1
    i2 j2 x2
    ...

where n is the number of dimensions and the i, j, x are triples with i, j integers with 0 <= i, j < n and x a float with x >= 0. This is interpreted as an nxn matrix with Aij = x. 

The error messages on parsing failure are currently not very good. Sorry. I'll fix that at some point.

# Output format
The output is to stdout and looks like the following:

Score: 12.845055
Optimal ordering: 11 1 2 [7 3] 8 || [14 9 12] [10 13] [5 0] 4 6

The || indicates the presence of a condorcet partition at that point. A bracketed set of indices such as [7 3] indicates a tie where the order of the elements in the brackets does not matter. 
