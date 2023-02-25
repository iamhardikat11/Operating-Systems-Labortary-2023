#include <bits/stdc++.h>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

// const char *shared_mem_name = "/my_shared_mem";
// const size_t shared_mem_size = 1024;

// int main()
// {
//     // Create shared memory object
//     int shared_mem_fd = shm_open(shared_mem_name, O_CREAT | O_RDWR, 0666);
//     if (shared_mem_fd == -1)
//     {
//         cerr << "Failed to create shared memory object" << endl;
//         exit(1);
//     }

//     // Resize shared memory
//     if (ftruncate(shared_mem_fd, shared_mem_size) == -1)
//     {
//         cerr << "Failed to resize shared memory" << endl;
//         exit(1);
//     }

//     // Map shared memory
//     char *shared_mem_ptr = static_cast<char *>(mmap(nullptr, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_fd, 0));
//     if (shared_mem_ptr == MAP_FAILED)
//     {
//         cerr << "Failed to map shared memory" << endl;
//         exit(1);
//     }

//     // Copy message to shared memory
//     const char *message = "Hello World";
//     strncpy(shared_mem_ptr, message, shared_mem_size);

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

//     return 0;
// }
int main() {
    // Open a shared memory object with read/write permissions
    int shm_fd = shm_open("/myshm", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        cerr << "Failed to open shared memory object" << endl;
        exit(EXIT_FAILURE);
    }
    
    // Set the size of the shared memory object to hold the map
    size_t size = sizeof(map<int, set<int>>*);
    if (ftruncate(shm_fd, size) == -1) {
        cerr << "Failed to set size of shared memory object" << endl;
        exit(EXIT_FAILURE);
    }
    
    // Map the shared memory object into the process's address space
    void* mem_ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (mem_ptr == MAP_FAILED) {
        cerr << "Failed to map shared memory object" << endl;
        exit(EXIT_FAILURE);
    }
    
    // Create a pointer to the map in shared memory
    map<int, set<int>>** map_ptr = static_cast<map<int, set<int>>**>(mem_ptr);
    
    // Initialize the map
    *map_ptr = new map<int, set<int>>();
    
    // Unmap the shared memory object
    if (munmap(mem_ptr, size) == -1) {
        cerr << "Failed to unmap shared memory object" << endl;
        exit(EXIT_FAILURE);
    }
    
    // Close the shared memory object
    // if (close(shm_fd) == -1) {
    //     cerr << "Failed to close shared memory object" << endl;
    //     exit(EXIT_FAILURE);
    // }
    
    return 0;
}