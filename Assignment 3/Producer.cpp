// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <vector>
// #include <map>
// #include <set>
// #include <random>
// #include <chrono>
// #include <cstring>
// #include <unistd.h>
// #include <sys/mman.h>
// #include <sys/stat.h>
// #include <sys/wait.h>
// #include <fcntl.h>

// using namespace std;
// const char *FILENAME = "facebook_combined.txt";
// const char *output_file = "output2.txt";
// string shared_mem_name = "/my_shared_mem1";
// int shared_mem_fd = 0;

// void printGraph(map<int, set<int>> *graph)
// {
//     ofstream file(output_file);
//     if (!file.is_open())
//     {
//         cerr << "Failed to open output file " << output_file << endl;
//         exit(1);
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

// // int main()
// // {
// //     shared_mem_fd = shm_open(shared_mem_name.c_str(), O_RDONLY, 0);
// //     if (shared_mem_fd == -1)
// //     {
// //         cerr << "Failed to open shared memory object" << endl;
// //         exit(1);
// //     }
// //     int NUM_NODES = 4039;
// //     size_t shared_mem_size1 = sizeof(map<int, set<int>>) + NUM_NODES * sizeof(set<int>);
// //     cout << shared_mem_size1 << endl;
// //     // Map shared memory
// //     map<int, set<int>> *graph1 = static_cast<map<int, set<int>> *>(mmap(nullptr, shared_mem_size1, PROT_READ, MAP_SHARED, shared_mem_fd, 0));
// //     if (graph1 == MAP_FAILED)
// //     {
// //         cerr << "Failed to map shared memory" << endl;
// //         exit(1);
// //     }
// //     cout << graph1->size() << endl;
// //     if (fork() == 0)
// //     {
// //         for (auto it : *graph1)
// //         {
// //             int node = it.first;
// //             set<int> neighbors = it.second;
// //             cout << "Node " << node << " has neighbors: ";
// //             for (int neighbor : neighbors)
// //                 cout << neighbor << " ";
// //             cout << endl;
// //         }
// //         exit(0);
// //     }
// //     int status;
// //     wait(&status);
// //     cout << status << endl;
// //     // graph1->operator[](4038).insert(0);
// //     // graph1->operator[](0).insert(4038);
// //     // printGraph(graph1);
// //     // Unmap shared memory
// //     if (munmap(graph1, sizeof(map<int, set<int>>) + NUM_NODES * sizeof(set<int>)) == -1)
// //     {
// //         cerr << "Failed to unmap shared memory" << endl;
// //         exit(1);
// //     }
// //     // Close shared memory object
// //     if (close(shared_mem_fd) == -1)
// //     {
// //         cerr << "Failed to close shared memory object" << endl;
// //         exit(1);
// //     }

// //     // Remove shared memory object
// //     if (shm_unlink(shared_mem_name.c_str()) == -1)
// //     {
// //         cerr << "Failed to remove shared memory object" << endl;
// //         exit(1);
// //     }
// //     return 0;
// // }
// int main() {
//     // Open shared memory object for reading and writing
//     shared_mem_fd = shm_open(shared_mem_name.c_str(), O_RDWR, 0);
//     if (shared_mem_fd == -1) {
//         cerr << "Failed to open shared memory object" << endl;
//         exit(1);
//     }

//     // Map shared memory
//     int NUM_NODES = 4039;
//     size_t shared_mem_size1 = sizeof(map<int, set<int>>) + NUM_NODES * sizeof(set<int>);
//     cout << shared_mem_size1 << endl;
//     map<int, set<int>> *graph1 = static_cast<map<int, set<int>> *>(mmap(nullptr, shared_mem_size1, PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_fd, 0));
//     if (graph1 == MAP_FAILED) {
//         cerr << "Failed to map shared memory" << endl;
//         exit(1);
//     }
//     cout << graph1->size() << endl;

//     // Create child process
//     pid_t pid = fork();
//     if (pid == -1) {
//         cerr << "Failed to fork" << endl;
//         exit(1);
//     } else if (pid == 0) {
//         // Child process
//         graph1->operator[](4038).insert(0);
//         graph1->operator[](0).insert(4038);
//         // exit(0);
//     } else {
//         // Parent process
//         // Wait for child to finish
//         int status;
//         wait(&status);
//         if (WIFEXITED(status)) {
//             cout << "Child process exited with status " << WEXITSTATUS(status) << endl;
//         } else {
//             cout << "Child process did not exit normally" << endl;
//         }

//         // Print graph and clean up shared memory
//         printGraph(graph1);
//         if (munmap(graph1, shared_mem_size1) == -1) {
//             cerr << "Failed to unmap shared memory" << endl;
//             exit(1);
//         }
//         if (close(shared_mem_fd) == -1) {
//             cerr << "Failed to close shared memory object" << endl;
//             exit(1);
//         }
//         if (shm_unlink(shared_mem_name.c_str()) == -1) {
//             cerr << "Failed to remove shared memory object" << endl;
//             exit(1);
//         }
//     }
//     return 0;
// }
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
    int NUM_NODES = 4039;
    int shm_id = shmget(IPC_PRIVATE, 1048576, 0666);
    struct shmid_ds shm_desc;
    if (shm_id < 0)
    {
        cout << "Error: could not create shared memory segment." << endl;
        return 1;
    }
    void *shm_ptr_p = shmat(shm_id, NULL, 0);
    if (shm_ptr_p == (void *)-1)
    {
        cout << "Error: could not attach shared memory segment." << endl;
        return 1;
    }

    // cast the pointer to a pointer of Node pointers
    Node **adj_list_p = static_cast<Node **>(shm_ptr_p);
    // traverse the linked list of node 0 and add a new node for 4038
    Node *node = new Node;
    node->data = 4038;
    node->next = adj_list_p[0];
    adj_list_p[0] = node;

    // traverse the linked list of node 4038 and add a new node for 0
    node = new Node;
    node->data = 0;
    node->next = adj_list_p[4038];
    adj_list_p[4038] = node;
    // pid_t pid = fork();
    // if(pid == 0)
    // {
    //     int x = 4039, y = 4040;
    //     cout << "Hello" << endl;
    //     // create a new node for b and add it to the linked list of a
    //     Node *node = new Node;
    //     node->data = y;
    //     node->next = adj_list_p[x];
    //     adj_list_p[a] = node;

    //     // create a new node for a and add it to the linked list of b
    //     node = new Node;
    //     node->data = x;
    //     node->next = adj_list_p[y];
    //     adj_list_p[b] = node;
    //     exit(0);
    // }
    // wait(NULL);
    cout << sizeof(adj_list_p) << endl;
    print(adj_list_p, NUM_NODES, "output4.txt");
    shmdt(shm_ptr_p);
    shmctl(shm_id, IPC_RMID, &shm_desc);
    return 0;
}