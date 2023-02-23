#include <bits/stdc++.h>
#include <fstream>
#include <random>
#include <algorithm>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

struct Edge {
    int x, y;
};

struct NodeDegree {
    int node;
    int degree;
};

bool compareDegrees(NodeDegree a, NodeDegree b) {
    return a.degree > b.degree;
}

int main() {
    // Open input file
    ifstream infile("facebook_combined.txt");
    if (!infile) {
        cerr << "Failed to open input file." << endl;
        return 1;
    }

    // Create shared memory for edges
    int fd = shm_open("/graph", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        cerr << "Failed to create shared memory." << endl;
        return 1;
    }

    // Set size of shared memory
    int num_edges = 0;
    while (infile.good()) {
        int x, y;
        infile >> x >> y;
        num_edges++;
    }
    if (ftruncate(fd, num_edges * sizeof(Edge)) == -1) {
        cerr << "Failed to set size of shared memory." << endl;
        return 1;
    }

    // Map shared memory to process address space
    Edge *edges = static_cast<Edge*>(mmap(NULL, num_edges * sizeof(Edge), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (edges == MAP_FAILED) 
    {
        cerr << "Failed to map shared memory." << endl;
        return 1;
    }

    // Read edges from input file and store in shared memory
    infile.clear();
    infile.seekg(0);
    int i = 0;
    while (infile.good()) {
        int x, y;
        infile >> x >> y;
        edges[i++] = {x, y};
    }

    // Create shared memory for node degrees
    int fd_degrees = shm_open("/degrees", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_degrees == -1) {
        cerr << "Failed to create shared memory." << endl;
        return 1;
    }

    // Set size of shared memory
    if (ftruncate(fd_degrees, num_edges * sizeof(NodeDegree)) == -1) {
        cerr << "Failed to set size of shared memory." << endl;
        return 1;
    }

    // Map shared memory to process address space
    NodeDegree *node_degrees = static_cast<NodeDegree*>(mmap(NULL, num_edges * sizeof(NodeDegree), PROT_READ | PROT_WRITE, MAP_SHARED, fd_degrees, 0));
    if (node_degrees == MAP_FAILED) {
        cerr << "Failed to map shared memory." << endl;
        return 1;
    }

    // Initialize node degrees
    map<int, int> degree_count;
    for (int i = 0; i < num_edges; i++) {
        int x = edges[i].x;
        int y = edges[i].y;
        degree_count[x]++;
        degree_count[y]++;
    }
    i = 0;
    for (auto& p : degree_count) 
        node_degrees[i++] = {p.first, p.second};
    sort(node_degrees, node_degrees + degree_count.size(), compareDegrees);
    for(int i=0;i<num_edges;i++)
    {
        cout << edges[i].x << " " << edges[i].y << endl;
    }    
    // cleanup
    munmap(node_degrees, sizeof(node_degrees));
    munmap(edges, sizeof(edges));
    shm_unlink("/graph");
    return 0;
}