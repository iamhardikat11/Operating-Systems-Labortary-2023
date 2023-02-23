#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

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
    key_t key_edges = ftok("/dev/null", 'A');
    int shmid_edges = shmget(230, sizeof(map<int, set<int>>), IPC_CREAT | 0666);
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
    for(int i = 0; i < 5; i++)
    {
        edges->insert(make_pair(i, vector<int>(5, 0)));
        // for(int i = 0; i < )
    }
    // Initialize edges

    // (*edges)[0] = {1, 2};
    // (*edges)[1] = {0, 2, 3};
    // (*edges)[2] = {0, 1, 3};
    // (*edges)[3] = {1, 2, 4};
    // (*edges)[4] = {3};

    // Create shared memory segment for node degrees
    // key_t key_degrees = ftok("/dev/null", 'B');
    // int shmid_degrees = shmget(key_degrees, sizeof(map<int, int>), IPC_CREAT | 0666);
    // if (shmid_degrees == -1)
    // {
    //     cerr << "Failed to create shared memory segment for node degrees." << endl;
    //     return 1;
    // }

    // Attach shared memory segment for node degrees
    // void *addr_degrees = shmat(shmid_degrees, NULL, 0);
    // if (addr_degrees == reinterpret_cast<void *>(-1))
    // {
    //     cerr << "Failed to attach shared memory segment for node degrees." << endl;
    //     return 1;
    // }

    // // Initialize node degrees
    // map<int, int> *node_degrees = static_cast<map<int, int> *>(addr_degrees);
    // for (int i = 0; i < 5; i++)
    // {
    //     int degree = (*edges)[i].size();
    //     (*node_degrees)[i] = degree;
    // }

    // // Sort node degrees in descending order
    // vector<NodeDegree> node_degrees_vec;
    // for (auto &p : *node_degrees)
    // {
    //     node_degrees_vec.push_back({p.first, p.second});
    // }
    // sort(node_degrees_vec.begin(), node_degrees_vec.end(), compareDegrees);

    // // Print graph and node degrees
    // cout << "Graph:" << endl;
    // for (int i = 0; i < 5; i++)
    // {
    //     cout << i << ": ";
    //     for (auto &j : (*edges)[i])
    //     {
    //         cout << j << " ";
    //     }
    //     cout << endl;
    // }
    // cout << "Degree:" << endl;
    // for (auto &p : node_degrees_vec)
    // {
    //     cout << p.node << ": " << p.degree << endl;
    // }
    shmdt(edges);
    // shmdt(addr_degrees);
    // shmctl(shmid_degrees, IPC_RMID, NULL);
    shmctl(shmid_edges, IPC_RMID, NULL);
    return 0;
}