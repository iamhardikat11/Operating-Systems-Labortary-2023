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
const char *output_file = "output2.txt";
string shared_mem_name = "/my_shared_mem1";
int shared_mem_fd = 0;

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

// int main()
// {
//     shared_mem_fd = shm_open(shared_mem_name.c_str(), O_RDONLY, 0);
//     if (shared_mem_fd == -1)
//     {
//         cerr << "Failed to open shared memory object" << endl;
//         exit(1);
//     }
//     int NUM_NODES = 4039;
//     size_t shared_mem_size1 = sizeof(map<int, set<int>>) + NUM_NODES * sizeof(set<int>);
//     cout << shared_mem_size1 << endl;
//     // Map shared memory
//     map<int, set<int>> *graph1 = static_cast<map<int, set<int>> *>(mmap(nullptr, shared_mem_size1, PROT_READ, MAP_SHARED, shared_mem_fd, 0));
//     if (graph1 == MAP_FAILED)
//     {
//         cerr << "Failed to map shared memory" << endl;
//         exit(1);
//     }
//     cout << graph1->size() << endl;
//     if (fork() == 0)
//     {
//         for (auto it : *graph1)
//         {
//             int node = it.first;
//             set<int> neighbors = it.second;
//             cout << "Node " << node << " has neighbors: ";
//             for (int neighbor : neighbors)
//                 cout << neighbor << " ";
//             cout << endl;
//         }
//         exit(0);
//     }
//     int status;
//     wait(&status);
//     cout << status << endl;
//     // graph1->operator[](4038).insert(0);
//     // graph1->operator[](0).insert(4038);
//     // printGraph(graph1);
//     // Unmap shared memory
//     if (munmap(graph1, sizeof(map<int, set<int>>) + NUM_NODES * sizeof(set<int>)) == -1)
//     {
//         cerr << "Failed to unmap shared memory" << endl;
//         exit(1);
//     }
//     // Close shared memory object
//     if (close(shared_mem_fd) == -1)
//     {
//         cerr << "Failed to close shared memory object" << endl;
//         exit(1);
//     }

//     // Remove shared memory object
//     if (shm_unlink(shared_mem_name.c_str()) == -1)
//     {
//         cerr << "Failed to remove shared memory object" << endl;
//         exit(1);
//     }
//     return 0;
// }
int main() {
    // Open shared memory object for reading and writing
    shared_mem_fd = shm_open(shared_mem_name.c_str(), O_RDWR, 0);
    if (shared_mem_fd == -1) {
        cerr << "Failed to open shared memory object" << endl;
        exit(1);
    }

    // Map shared memory
    int NUM_NODES = 4039;
    size_t shared_mem_size1 = sizeof(map<int, set<int>>) + NUM_NODES * sizeof(set<int>);
    cout << shared_mem_size1 << endl;
    map<int, set<int>> *graph1 = static_cast<map<int, set<int>> *>(mmap(nullptr, shared_mem_size1, PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_fd, 0));
    if (graph1 == MAP_FAILED) {
        cerr << "Failed to map shared memory" << endl;
        exit(1);
    }
    cout << graph1->size() << endl;

    // Create child process
    pid_t pid = fork();
    if (pid == -1) {
        cerr << "Failed to fork" << endl;
        exit(1);
    } else if (pid == 0) {
        // Child process
        graph1->operator[](4038).insert(0);
        graph1->operator[](0).insert(4038);
        // exit(0);
    } else {
        // Parent process
        // Wait for child to finish
        int status;
        wait(&status);
        if (WIFEXITED(status)) {
            cout << "Child process exited with status " << WEXITSTATUS(status) << endl;
        } else {
            cout << "Child process did not exit normally" << endl;
        }

        // Print graph and clean up shared memory
        printGraph(graph1);
        if (munmap(graph1, shared_mem_size1) == -1) {
            cerr << "Failed to unmap shared memory" << endl;
            exit(1);
        }
        if (close(shared_mem_fd) == -1) {
            cerr << "Failed to close shared memory object" << endl;
            exit(1);
        }
        if (shm_unlink(shared_mem_name.c_str()) == -1) {
            cerr << "Failed to remove shared memory object" << endl;
            exit(1);
        }
    }
    return 0;
}