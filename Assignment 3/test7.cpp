#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <random>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>

using namespace std;

// Structure to store degree of each node in the graph
struct NodeDegree
{
    int node;   // Node number
    int degree; // Degree of the node
};

// Function to compare NodeDegree objects by degree in descending order
bool compareDegrees(NodeDegree a, NodeDegree b)
{
    return a.degree > b.degree;
}

int main()
{
    // Open file containing edges
    ifstream infile("facebook_combined.txt");
    if (!infile.is_open()) {
        cerr << "Failed to open file." << endl;
        return 1;
    }

    // Create shared memory segment for edges
    int fd_edges = shm_open("/edges", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_edges == -1) {
        cerr << "Failed to create shared memory segment for edges." << endl;
        return 1;
    }

    // Set the size of shared memory segment for edges
    size_t size_edges = sizeof(map<int, set<int>>);
    if (ftruncate(fd_edges, size_edges) == -1) {
        cerr << "Failed to set the size of shared memory segment for edges." << endl;
        return 1;
    }

    // Map shared memory segment for edges into the process's address space
    map<int, set<int>> *edges = static_cast<map<int, set<int>> *>(mmap(NULL, size_edges, PROT_READ | PROT_WRITE, MAP_SHARED, fd_edges, 0));
    if (edges == MAP_FAILED) {
        cerr << "Failed to map shared memory segment for edges." << endl;
        return 1;
    }

    // Create shared memory segment for node degrees
    int fd_degrees = shm_open("/degrees", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_degrees == -1) {
        cerr << "Failed to create shared memory segment for node degrees." << endl;
        return 1;
    }

    // Set the size of shared memory segment for node degrees
    size_t size_degrees = sizeof(map<int, int>);
    if (ftruncate(fd_degrees, size_degrees) == -1) {
        cerr << "Failed to set the size of shared memory segment for node degrees." << endl;
        return 1;
    }

    // Map shared memory segment for node degrees into the process's address space
    map<int, int> *node_degrees = static_cast<map<int, int> *>(mmap(NULL, size_degrees, PROT_READ | PROT_WRITE, MAP_SHARED, fd_degrees, 0));
    if (node_degrees == MAP_FAILED) {
        cerr << "Failed to map shared memory segment for node degrees." << endl;
        return 1;
    }

    // Initialize edges and node degrees
    string line;
    int x, y;
    while (getline(infile, line)) {
        istringstream iss(line);
        if (!(iss >> x >> y)) {
            cerr << "Failed to read edge from file." << endl;
            return 1;
        }
        (*edges)[x].insert(y);
        (*edges)[y].insert(x);
        (*node_degrees)[x] = (*edges)[x].size();
        (*node_degrees)[y] = (*edges)[y].size();
    }
    
}
