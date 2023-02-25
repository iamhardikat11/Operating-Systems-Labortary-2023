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

void printMap(map<int, set<int>> *myMap)
{
    for (auto it = myMap->begin(); it != myMap->end(); ++it)
    {
        cout << it->first << ": ";
        for (auto it_set = it->second.begin(); it_set != it->second.end(); ++it_set)
        {
            cout << *it_set << " ";
        }
        cout << endl;
    }
}

int main()
{
    // open shared memory object
    const char *name = "/my_shared";
    int shm_fd = shm_open(name, O_RDWR, 0666);

    // map the shared memory object to memory
    int SIZE = sizeof(map<int, set<int>>);
    void *ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    auto myMap = static_cast<map<int, set<int>> *>(ptr);

    cout << myMap->size() << endl;
    // acquire the semaphore
    // sem_t *sem = sem_open("/my_semaphore", 0);
    // sem_wait(sem);

    // update the map
    myMap->operator[](1).insert(4);
    // myMap->insert({2, {2}});
    // myMap->at(2).insert(3);
    // myMap->at(2).insert(4);

    // release the semaphore
    // sem_post(sem);

    // print the updated map
    // printMap(myMap);

    // release the shared memory object
    munmap(ptr, SIZE);
    close(shm_fd);

    cout << "Shared memory object accessed and updated" << endl;

    return 0;
}