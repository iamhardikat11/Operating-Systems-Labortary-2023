#include <bits/stdc++.h>
#include <sys/wait.h>
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
    srand((unsigned long int)time(NULL));
    // Create shared memory segment for edges
    key_t key_edges = ftok("shmfile", 69);
    int shmid_edges = shmget(key_edges, sizeof(int *) * 2 * 1024 * 1024, IPC_CREAT | 0666);
    if (shmid_edges == -1)
    {
        cerr << "Failed to create shared memory segment for edges." << endl;
        return 1;
    }

    // Attach shared memory segment for edges
    int **edges = static_cast<int **>(shmat(shmid_edges, NULL, 0));
    if (edges == reinterpret_cast<int **>(-1))
    {
        cerr << "Failed to attach shared memory segment for edges." << endl;
        return 1;
    }
    FILE *fp;
    fp = fopen("facebook_combined.txt", "r");
    if (fp == NULL)
    {
        perror("Error in fopen");
        shmctl(shmid_edges, IPC_RMID, NULL);
        exit(1);
    }
    int n = 0;
    vector<pair<int, int>> edges_graph;
    int u, v;
    while (fscanf(fp, "%d %d", &u, &v) != EOF)
    {
        edges_graph.push_back(make_pair(u, v));
        n = max(n, max(u, v));
    }
    fclose(fp);
    int number_of_nodes = n+1;
    int number_of_edges = edges_graph.size();
    // Initialize edges
    for (int i = 0; i < number_of_nodes; i++)
    {
        edges[i] = new int[n];
        for (int j = 0; j < number_of_nodes; j++)
            edges[i][j] = 0;
    }
    
    cout <<  number_of_edges << " " << number_of_nodes << endl;
    for(int i=0;i<edges_graph.size();i++)
    {
        edges[edges_graph[i].first][edges_graph[i].second] = 1;
        edges[edges_graph[i].second][edges_graph[i].first] = 1;
    }
    // edges[0][1] = edges[0][2] = 1;
    // edges[1][0] = edges[1][2] = edges[1][3] = 1;
    // edges[2][0] = edges[2][1] = edges[2][3] = 1;
    // edges[3][1] = edges[3][2] = edges[3][4] = 1;
    // edges[4][3] = 1;

    // Create shared memory segment for node degrees
    // key_t key_degrees = ftok("/dev/null", 'B');
    // int shmid_degrees = shmget(key_degrees, sizeof(vector<NodeDegree>), IPC_CREAT | 0666);
    // if (shmid_degrees == -1)
    // {
    //     cerr << "Failed to create shared memory segment for node degrees." << endl;
    //     return 1;
    // }

    // // Attach shared memory segment for node degrees
    // void *addr_degrees = shmat(shmid_degrees, NULL, 0);
    // if (addr_degrees == reinterpret_cast<void *>(-1))
    // {
    //     cerr << "Failed to attach shared memory segment for node degrees." << endl;
    //     return 1;
    // }

    // // Initialize node degrees
    // vector<NodeDegree> *node_degrees = static_cast<vector<NodeDegree> *>(addr_degrees);
    // node_degrees->resize(number_of_nodes);
    // for (int i = 0; i < number_of_nodes; i++)
    // {
    //     int degree = 0;
    //     for (int j = 0; j < number_of_nodes; j++)
    //         degree += edges[i][j];
    //     (*node_degrees)[i] = {i, degree};
    // }
    // sort(node_degrees->begin(), node_degrees->end(), compareDegrees);
    // cout << "Graph:" << endl;
    // for (int i = 0; i < number_of_nodes; i++)
    // {
    //     cout << i << ": ";
    //     for (int j = 0; j < number_of_nodes; j++)
    //     {
    //         cout << edges[i][j] << " ";
    //     }
    //     cout << endl;
    // }
    // cout << "Degree:" << endl;
    // for (auto &p : *node_degrees)
    // {
    //     cout << p.node << ": " << p.degree << endl;
    // }
    shmdt(edges);
    // shmdt(addr_degrees);
    // shmctl(shmid_degrees, IPC_RMID, NULL);
    shmctl(shmid_edges, IPC_RMID, NULL);
    return 0;
}