how to run:
make Makefile
g++ -c goodmalloc.cpp
ar -rcs libgoodmalloc.a goodmalloc.o
g++ -o merge mergesort.c libgoodmalloc.a -pthread