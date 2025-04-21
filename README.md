# Adaptive Radix Tree

## Description

The project implements an in-memory indexing data structure called Adaptive Radix Tree(ART). ART supports fast point/range query and prefix matching with a relatively low space consumption.  
Compared to a trie or radix tree, ART optimizes space utilization by introducing four different node sizes and having a node expand or shrink as needed. ART also uses SIMD instructions provided by modern CPUs to speed up the search process inside a node.  
Please refer to the paper listed in Reference for more details.

## Structure

- **/tests**: unit tests using GoogleTest framework
- **/include**:
  - **/art**: library implementation
  - `art_printer.hpp`: a helper class to print the whole tree
  - `art.hpp`: header to include
- `example.cpp`: example code

## Usage

1. Default build
   By default, the example would be release build and unit tests would be debug build
  - test and example binary is under /build/bin/
```bash
  # At project root directory, do the following:
  mkdir build
  cmake -S . -B build
  cmake --build build
```

## Reference

[The Adaptive Radix Tree:ARTful Indexing for Main-Memory Databases](https://db.in.tum.de/~leis/papers/ART.pdf)
