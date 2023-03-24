# Compiler settings
# To Compile:-
# make
# ./main
# To Clean 
# make clean
CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra -pedantic

all : main

main : main.cpp room.cpp guest.cpp cleaner.cpp
	$(CXX) $(CXXFLAGS) main.cpp room.cpp guest.cpp cleaner.cpp -o main -lpthread

clean :
	rm -f *.o main
