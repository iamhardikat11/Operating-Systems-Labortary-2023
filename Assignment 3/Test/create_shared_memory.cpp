#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <semaphore.h>
#include <map>
#include <set>

using namespace std;

void printMap(map<int, set<int>>* myMap) {
    for (auto it = myMap->begin(); it != myMap->end(); ++it) {
        cout << it->first << ": ";
        for (auto it_set = it->second.begin(); it_set != it->second.end(); ++it_set) {
            cout << *it_set << " ";
        }
        cout << endl;
    }
}

int main() {
    // create shared memory object
    int SIZE = sizeof(map<int, set<int>>);
    const char* name = "/my_shared";
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SIZE);
    
    // map the shared memory object to memory
    void* ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    auto myMap = new (ptr) map<int, set<int>>;

    // initialize the map
    myMap->insert({1, {1}});
    myMap->at(1).insert(2);
    myMap->at(1).insert(3);

    // release the shared memory object
    // munmap(ptr, SIZE);
    // close(shm_fd);

    cout << "Shared memory object created and initialized" << endl;

    // create semaphore
    // sem_t* sem = sem_open("/my_semaphore", O_CREAT, 0666, 1);
    // sem_unlink("/my_semaphore");

    return 0;
}