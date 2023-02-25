#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <random>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;
const char *FILENAME = "facebook_combined.txt";
const char *output_file = "output3.txt";
string shared_mem_name = "/my_shared_mem1";
int shared_mem_fd = 0;
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

// Function to read edges from file and store in shared memory
void readEdgesToSharedMemory(const char *filename, int num_edges, int num_nodes, map<int, set<int>> *graph)
{
    // Open file
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Failed to open file " << filename << endl;
        exit(1);
    }
    // Read edges and add to graph
    int x, y;
    for (int i = 0; i < num_edges; i++)
    {
        file >> x >> y;
        if (x >= num_nodes || y >= num_nodes)
        {
            cerr << "Invalid edge (" << x << "," << y << ")" << endl;
            exit(1);
        }
        graph->operator[](x).insert(y);
        graph->operator[](y).insert(x);
    }
    // Close file
    file.close();
}

// Function to create shared memory for graph
map<int, set<int>> *createSharedMemoryForGraph(int num_nodes)
{
    // Open shared memory object
    shared_mem_fd = shm_open(shared_mem_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (shared_mem_fd == -1)
    {
        cerr << "Failed to create shared memory object" << endl;
        exit(1);
    }

    // Resize shared memory
    size_t shared_mem_size = sizeof(map<int, set<int>>) + num_nodes * sizeof(set<int>);
    if (ftruncate(shared_mem_fd, shared_mem_size) == -1)
    {
        cerr << "Failed to resize shared memory" << endl;
        exit(1);
    }

    // Map shared memory
    map<int, set<int>> *graph = static_cast<map<int, set<int>> *>(mmap(nullptr, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_fd, 0));
    if (graph == MAP_FAILED)
    {
        cerr << "Failed to map shared memory" << endl;
        exit(1);
    }

    // Initialize graph
    new (graph) map<int, set<int>>();
    for (int i = 0; i < num_nodes; i++)
        new (&(graph->operator[](i))) set<int>();
    // graph->operator[](4038).insert(0);
    // graph->operator[](0).insert(4038);
    return graph;
}
void printGraph(map<int, set<int>> *graph)
{
    ofstream file(output_file);
    if (!file.is_open())
    {
        cerr << "Failed to open output file " << output_file << endl;
        exit(1);
    }
    for (const auto &kv : *graph)
    {
        int node = kv.first;
        file << node << ": ";
        for (int neighbor : kv.second)
            file << neighbor << " ";
        file << endl;
    }
}

int main()
{
    srand((unsigned long int)time(NULL));
    ifstream file(FILENAME);
    if (!file.is_open())
    {
        cerr << "Failed to open file " << FILENAME << endl;
        exit(1);
    }
    // Read edges and add to graph
    int x, y, NUM_NODES = 0, NUM_EDGES = 0;
    while (file >> x >> y)
    {
        NUM_NODES = max(NUM_NODES, max(x, y));
        NUM_EDGES++;
    }
    NUM_NODES++;
    // Close file
    file.close();
    // Create shared memory for graph
    map<int, set<int>> *graph = createSharedMemoryForGraph(NUM_NODES);
    // Read edges from file and store in shared memory
    readEdgesToSharedMemory(FILENAME, NUM_EDGES, NUM_NODES, graph);
    printGraph(graph);

    // Unmap shared memory
    size_t size_final = sizeof(map<int, set<int>>) + NUM_NODES * sizeof(set<int>);
    cout << size_final << endl;
    // if (munmap(graph, size_final) == -1)
    // {
    //     cerr << "Failed to unmap shared memory" << endl;
    //     exit(1);
    // }
    // // Close shared memory object
    // if(close(shared_mem_fd) == -1)
    // {
    //     cerr << "Failed to close shared memory object" << endl;
    //     exit(1);
    // }
    // Open a ne wshared memory object
    // int shared_mem_fd1 = shm_open(shared_mem_name.c_str(), O_CREAT | O_RDWR, 0666);
    // if (shared_mem_fd == -1)
    // {
    //     cerr << "Failed to create shared memory object" << endl;
    //     exit(1);
    // }
    // // Resize shared memory
    // size_t shared_mem_size1 = sizeof(map<int, set<int>>) + NUM_NODES * sizeof(set<int>);
    // // Map shared memory
    // map<int, set<int>> *graph1 = static_cast<map<int, set<int>> *>(mmap(nullptr, shared_mem_size1, PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_fd, 0));
    // if (graph1 == MAP_FAILED)
    // {
    //     cerr << "Failed to map shared memory" << endl;
    //     exit(1);
    // }
    // // graph1->operator[](4038).insert(0);
    // graph1->operator[](0).insert(4038);

    // Unmap shared memory
    // munmap(graph1, sizeof(map<int, set<int>>) + NUM_NODES * sizeof(set<int>));

    // Close shared memory file descriptor
    // close(shared_mem_fd1);
    // unlink(shared_mem_name.c_str());
    // while (1)
    // {
    //     ;
    // }
    return 0;
}