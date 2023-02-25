#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
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

struct Graph {
    int num_nodes;
    vector<int> *adj_list;
};

// void printGraph(map<int, set<int>> *graph, const char* output_file);
int main() {
    ifstream infile("facebook_combined.txt");

    // Read the number of nodes and edges
    int num_nodes, num_edges;
    infile >> num_nodes >> num_edges;

    // Allocate memory for the graph
    Graph *graph_ptr;
    int shm_id = shmget(230, 200*sizeof(Graph), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }
    graph_ptr = (Graph *) shmat(shm_id, NULL, 0);
    graph_ptr->num_nodes = num_nodes;
    graph_ptr->adj_list = new vector<int>[num_nodes];

    // Read the edges and populate the graph
    int u, v;
    for (int i = 0; i < num_edges; i++) {
        infile >> u >> v;
        graph_ptr->adj_list[u].push_back(v);
        graph_ptr->adj_list[v].push_back(u);
    }
    // Spawn a child process to add the extra edge
    if (!fork()) {
        // Attach to the shared memory
        // Graph *child_graph_ptr = (Graph *) shmat(shm_id, NULL, 0);
        // Add the extra edge
        graph_ptr->adj_list[4038].push_back(0);
        graph_ptr->adj_list[0].push_back(4038);
        // Detach from the shared memory
        shmdt(graph_ptr);
        exit(0);
    }

    // Wait for the child process to finish
    wait(NULL);

    // Print the graph
    for (int i = 0; i < num_nodes; i++) {
        cout << i << ": ";
        for (auto j : graph_ptr->adj_list[i]) {
            cout << j << " ";
        }
        cout << endl;
    }

    // Detach and remove the shared memory
    shmdt(graph_ptr);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
// void printGraph(Graph* graph, const char* output_file)
// {
//     ofstream file(output_file);
//     if (!file.is_open())
//     {
//         cerr << "Failed to open output file " << output_file << endl;
//         exit(1);
//     }
//     for (int i = 0; i < num_nodes; i++) {
//         cout << i << ": ";
//         for (auto j : graph_ptr->adj_list[i]) {
//             cout << j << " ";
//         }
//         cout << endl;
//     }
//     for (const auto &kv : *graph)
//     {
//         int node = kv.first;
//         file << node << ": ";
//         for (int neighbor : kv.second)
//             file << neighbor << " ";
//         file << endl;
//     }
// }