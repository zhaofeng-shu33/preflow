# Maximum Flow Algorithm (Preflow)
Implementation of preflow algorithm with a different 
**active node selection rules**.

[![Travis](https://travis-ci.com/zhaofeng-shu33/preflow.svg?branch=master)](
    https://travis-ci.com/zhaofeng-shu33/preflow)

## How to use
After successful compilation and link, you get an executable program called `lgf_compute`. You should use an LEMON graph file (digraph)
as input to this program, and the program print out the caculated result to the terminal. 
For example,  using provided test graph file, type `./lgf_compute --filename test.lgf` to finish the computation.

Selection Rules:

- Highest Relabel
- FIFO
- Relabel to front

Parallel Implementation:

- generic parallel