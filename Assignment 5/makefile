# Compiler settings
CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra -pedantic

all : main

main : main.cpp room.cpp guest.cpp cleaner.cpp
	$(CXX) $(CXXFLAGS) main.cpp room.cpp guest.cpp cleaner.cpp -o main -lpthread

clean :
	rm -f *.o main

# Files
# SRCS=room.cpp guest.cpp cleaner.cpp main.cpp
# OBJS=$(SRCS:.cpp=.o)
# EXEC=main

# # Targets
# all: $(EXEC)

# $(EXEC): $(OBJS)
# 	$(CXX) $(CXXFLAGS) $^ -o $@

# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# .PHONY: clean
# clean:
# 	rm -f $(OBJS) $(EXEC)