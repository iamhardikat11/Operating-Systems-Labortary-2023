#include <bits/stdc++.h>
#include <fstream>
#include <cstring>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
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
// define a struct to represent a node in the linked list
struct Node
{
    int data;
    Node *next;
};
void print(Node **adj_list, int n, const char *file_name)
{
    // open file for writing
    std::ofstream out_file(file_name);

    // loop through each node
    for (int i = 0; i < n; i++)
    {
        out_file << i << ": ";

        // loop through the neighbors of the node and print them
        Node *curr = adj_list[i];
        while (curr != NULL)
        {
            out_file << curr->data << " ";
            curr = curr->next;
        }

        out_file << std::endl;
    }
    // close file
    out_file.close();
}

int main()
{
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

    // open the file containing the edges of the graph
    ifstream infile(FILENAME);
    if (!infile.is_open())
    {
        cout << "Error: could not open file." << endl;
        return 1;
    }
    // create a shared memory segment of size 1MB

    int shm_id = shmget(IPC_PRIVATE, 1048576, IPC_CREAT | 0666);
    if (shm_id < 0)
    {
        cout << "Error: could not create shared memory segment." << endl;
        return 1;
    }

    // attach the shared memory segment to a pointer
    void *shm_ptr = shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1)
    {
        cout << "Error: could not attach shared memory segment." << endl;
        return 1;
    }

    // cast the pointer to a pointer of Node pointers
    Node **adj_list = static_cast<Node **>(shm_ptr);

    // initialize the linked lists to NULL
    for (int i = 0; i < NUM_NODES; i++) 
    {
        adj_list[i] = (Node *)shmat(shm_id, NULL, 0);
        adj_list[i] = NULL;
    }
    // read the edges from the file and create the adjacency list
    int a, b;
    while (infile >> a >> b)
    {
        // create a new node for b and add it to the linked list of a
        Node *node = new Node;
        node->data = b;
        node->next = adj_list[a];
        adj_list[a] = node;

        // create a new node for a and add it to the linked list of b
        node = new Node;
        node->data = a;
        node->next = adj_list[b];
        adj_list[b] = node;
    }
    print(adj_list, NUM_NODES, "output1.txt");
    // detach the shared memory segment
    for(int i=0;i<NUM_NODES;i++)
        shmdt(adj_list[i]);
    shmdt(adj_list);
    // delete the shared memory segment
    // shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}
