#include <iostream>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <map>
#include <set>

using namespace std;

// define the name of the semaphore
const char* SEM_NAME = "/my_semaphore";

// function to print the graph
void print(map<int, set<int>>* graph_ptr)
{
    for (auto it = graph_ptr->begin(); it != graph_ptr->end(); it++)
    {
        int node = it->first;
        set<int> edges = it->second;
        cout << "Node " << node << ": ";
        for (auto it2 = edges.begin(); it2 != edges.end(); it2++)
        {
            cout << *it2 << " ";
        }
        cout << endl;
    }
}

int main()
{
    const char* shm_name = "/my_shared_memory"; // name of shared memory object
    const int shm_size = 1024; // size of shared memory object
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666); // create shared memory object
    ftruncate(shm_fd, shm_size); // set size of shared memory object
    void* shm_ptr = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // map shared memory object to a pointer

    // initialize graph in shared memory using placement new
    map<int, set<int>>* graph_ptr = new(shm_ptr) map<int, set<int>>();

    // create the semaphore
    sem_t* sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);

    // access graph from different processes by mapping shared memory object to a pointer
    void* shm_ptr2 = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    map<int, set<int>>* graph_ptr2 = static_cast<map<int, set<int>>*>(shm_ptr2);

    // read graph from file
    ifstream infile("facebook_combined.txt");
    int u, v;
    cout << 1 << endl;
    while (infile >> u >> v)
    {
        // add edge to graph
        (*graph_ptr)[u].insert(v);
    }
    cout << 2 << endl;
    print(graph_ptr);
    
    if(fork()==0)
    {
        // wait for the semaphore
        sem_wait(sem);

        // add node with edge to graph
        (*graph_ptr2)[4039].insert(0);

        // release the semaphore
        sem_post(sem);

        exit(0);
    }
    wait(NULL);

    // wait for the semaphore
    sem_wait(sem);

    print(graph_ptr);

    // release the semaphore
    sem_post(sem);

    // cleanup
    munmap(shm_ptr, shm_size);
    munmap(shm_ptr2, shm_size);
    shm_unlink(shm_name);
    sem_unlink(SEM_NAME);
    return 0;
}
