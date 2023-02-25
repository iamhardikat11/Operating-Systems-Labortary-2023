// #include <iostream>
// #include <cstring>
// #include <sys/mman.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <unistd.h>

// using namespace std;

// const char *shared_mem_name = "/my_shared_mem";
// const size_t shared_mem_size = 1024;

// int main()
// {
//     // Open shared memory object
//     int shared_mem_fd = shm_open(shared_mem_name, O_RDONLY, 0666);
//     if (shared_mem_fd == -1)
//     {
//         cerr << "Failed to open shared memory object" << endl;
//         exit(1);
//     }

//     // Map shared memory
//     char *shared_mem_ptr = static_cast<char *>(mmap(nullptr, shared_mem_size, PROT_READ, MAP_SHARED, shared_mem_fd, 0));
//     if (shared_mem_ptr == MAP_FAILED)
//     {
//         cerr << "Failed to map shared memory" << endl;
//         exit(1);
//     }

//     // Print message from shared memory
//     cout << shared_mem_ptr << endl;

//     // Unmap shared memory
//     if (munmap(shared_mem_ptr, shared_mem_size) == -1)
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
//     if (shm_unlink(shared_mem_name) == -1)
//     {
//         cerr << "Failed to remove shared memory object" << endl;
//         exit(1);
//     }

//     return 0;
// }
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <random>
#include <map>
#include <set>

using namespace std;

int main() {
    // Open the shared memory object with read/write permissions
    int shm_fd = shm_open("/myshm", O_RDWR, 0666);
    if (shm_fd == -1) {
        cerr << "Failed to open shared memory object" << endl;
        exit(EXIT_FAILURE);
    }
    
    // Map the shared memory object into the process's address space
    int size = sizeof(map<int, set<int>>*);
    void* mem_ptr = mmap(NULL, sizeof(map<int, set<int>>*), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (mem_ptr == MAP_FAILED) {
        cerr << "Failed to map shared memory object" << endl;
        exit(EXIT_FAILURE);
    }
    
    // Create a pointer to the map in shared memory
    map<int, set<int>>** map_ptr = static_cast<map<int, set<int>>**>(mem_ptr);
    cout << (*(*map_ptr)).size() << endl;   
    // Write some random numbers to the map
    // random_device rd;
    // mt19937 gen(rd());
    // uniform_int_distribution<> dis(1, 100);
    // for (int i = 0; i < 10; ++i) {
    //     int key = dis(gen);
    //     int value = dis(gen);
    //     (*(*map_ptr))[key].insert(value);
    // }
    
    // Unmap the shared memory object
    if (munmap(mem_ptr, size) == -1) {
        cerr << "Failed to unmap shared memory object" << endl;
        exit(EXIT_FAILURE);
    }
    
    // Close the shared memory object
    if (close(shm_fd) == -1) {
        cerr << "Failed to close shared memory object" << endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}