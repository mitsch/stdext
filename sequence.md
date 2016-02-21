Sequence is a module for combination of operations for some arbitrary values. The framework is written in C++ (standard 2014). It is motivated by the Haskell module [Data.List](https://hackage.haskell.org/package/base-4.8.2.0/docs/Data-List.html). It aims to provide following features:
* Lazy evaluation
* Easy-To-Use and Easy-To-Understand by simple combinations
* Constant data behaviour
* Efficiency

The most important feature is the Easy-To-Use and Easy-To-Understand. Complex looping with temporary variables should be avoided and the basic understanding of algorithms should be cleary to spot. Therefore, simple combinators, called sequencers, support basic but powerful algorithms which can be put together.

## Sequence
A sequence is a definition of values with a specified order. The definition can be of mathematical nature or just raw values. Important is that these values have an order in which they can be accessed. A simple sequence would be the natural numbers, $$\mathbb{N} = \{1, 2, \dots\}$$. They can be seen as distinct sets. Either are they finite (or bounded) or infinite (unbounded). Additionally, elements in these sequences can be accessed without  having to traverse through all its predecessors or they cannot. These two variations lead to four concepts of sequences:
* directly accessible and finite sequences
* not directly accessible and finite sequences
* directly accessible and infinite sequences
* not directly accessible and infinite sequences






