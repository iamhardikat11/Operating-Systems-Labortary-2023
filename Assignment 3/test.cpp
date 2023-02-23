#include <iostream>
#include <queue>
#include <utility>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
void print(priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>>* pq_ptr2)
{
    // print elements from priority queue
    while (!pq_ptr2->empty())
    {
        cout << pq_ptr2->top().first << " " << pq_ptr2->top().second << endl;
        pq_ptr2->pop();
    }
}
int main()
{
    const char* shm_name = "/my_shared_memory"; // name of shared memory object
    const int shm_size = 1024; // size of shared memory object
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666); // create shared memory object
    ftruncate(shm_fd, shm_size); // set size of shared memory object
    void* shm_ptr = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // map shared memory object to a pointer

    // initialize priority queue in shared memory using placement new
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>>* pq_ptr = new(shm_ptr) priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>>();

    // access priority queue from different processes by mapping shared memory object to a pointer
    void* shm_ptr2 = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>>* pq_ptr2 = static_cast<priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>>*>(shm_ptr2);

    // add elements to priority queue
    pq_ptr->push(make_pair(3, 5));
    pq_ptr->push(make_pair(1, 7));
    pq_ptr->push(make_pair(2, 6));

    if(fork()==0)
    {
        pq_ptr2->push(make_pair(9,10));
        // exit(0);
    }
    wait(NULL);
    print(pq_ptr);
    // cleanup
    munmap(shm_ptr, shm_size);
    munmap(shm_ptr2, shm_size);
    shm_unlink(shm_name);
    return 0;
}
