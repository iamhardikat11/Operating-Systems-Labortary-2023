#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

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
    // Create shared memory segment for edges
    key_t key_edges = ftok("/my_shared_mem", 'A');
    int shmid_edges = shmget(key_edges, sizeof(map<int, vector<int>>), IPC_CREAT | 0666);
    if (shmid_edges == -1)
    {
        cerr << "Failed to create shared memory segment for edges." << endl;
        return 1;
    }

    // Attach shared memory segment for edges
    map<int, vector<int>> *edges = static_cast<map<int, vector<int>> *>(shmat(shmid_edges, NULL, 0));
    if (edges == reinterpret_cast<map<int, vector<int>> *>(-1))
    {
        cerr << "Failed to attach shared memory segment for edges." << endl;
        return 1;
    }

    // Read edges from input.txt and store them in the shared memory
    ifstream input("facebook_combined.txt");
    if (!input)
    {
        cerr << "Failed to open input file." << endl;
        return 1;
    }

    int x, y;
    while (input >> x >> y)
    {
        (*edges)[x].push_back(y);
        (*edges)[y].push_back(x);
    }

    // Create shared memory segment for node degrees
    key_t key_degrees = ftok("/dev/null", 'B');
    int shmid_degrees = shmget(key_degrees, sizeof(map<int,int>), IPC_CREAT | 0666);
    if (shmid_degrees == -1)
    {
        cerr << "Failed to create shared memory segment for node degrees." << endl;
        return 1;
    }

    // Attach shared memory segment for node degrees
    map<int,int> *node_degrees = static_cast<map<int,int> *>(shmat(shmid_degrees, NULL, 0));
    if (node_degrees == reinterpret_cast<map<int,int> *>(-1))
    {
        cerr << "Failed to attach shared memory segment for node degrees." << endl;
        return 1;
    }

    // Initialize node degrees
    for (auto const& x : *edges) {
        (*node_degrees)[x.first] = x.second.size();
    }

    // Create child process
    pid_t pid = fork();
    if (pid < 0)
    {
        cerr << "Failed to fork." << endl;
        return 1;
    }
    else if (pid == 0) // Child process
    {
        // Wait for 50 seconds
        sleep(50);

        // Generate random number of nodes to add to the graph
        srand(time(0));
        int m = rand() % 21 + 10;

        // Generate random number of edges to add to each node
        int r = rand() % 20 + 1;

        // Find nodes with highest degree
        vector<NodeDegree> degrees;
        // for (auto const& x : *node_degrees
        exit(0);
    }
    else
    {
        ;
    }
    wait(NULL);
    shmdt(edges);
    shmdt(node_degrees);
    shmctl(shmid_edges, IPC_RMID, NULL);
    shmctl(shmid_degrees, IPC_RMID, NULL);
    return 0;
}