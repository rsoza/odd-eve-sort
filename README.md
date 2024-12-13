# Odd Even Transposition Sort

This repo exploits the use of parallelization in sorting algorithms through odd-even transposition sort. This algorithm is a variant of bubble sort but has more opportunities to parallelism by decoupling compare-swaps in an even and odd fashion. When splitting the processes between even and odd while matching its partner process the algorithm compares and swap elements accordingly through a sequence of phases. The problem with this is that the implementation of MPI suffers from safety, that is, it is prone to hanging or crashing because of its even and odd structure since the bare bone program works effectively with even processes versus odd.

```
g++ odd_even_sort_serial.cpp -o odd_even_sort_serial
./odd_even_sort_serial 10000 1 10000
mpic++ odd_even_sort_parallel.cpp -o odd_even_sort_parallel
mpiexec --oversubscribe -n # ./odd_even_sort_parallel 10000
```
