# Sequence
Sequence is a small library for combining operations on some arbitrary values. The framework is written in C++ (standard 2014). It is motivated by the Haskell module [Data.List](https://hackage.haskell.org/package/base-4.8.2.0/docs/Data-List.html). It aims to provide following features:
* Lazy evaluation
* Easy-To-Use and Easy-To-Understand by simple combinations
* Constant data behaviour
* Efficiency

The most important feature is the Easy-To-Use and Easy-To-Understand. Complex looping with temporary variables should be avoided and the basic understanding of algorithms should be cleary to spot. Therefore, simple combinators, called sequencers, have to be formulated to support the most general use.