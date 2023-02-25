#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <map>
#include <sys/wait.h>
#include <set>

using namespace std;

void printMap(map<int, set<int>>* myMap);

int main() {
    const char* name = "/my_shared_memory";
    const int SIZE = sizeof(map<int, set<int>>);

    // create the shared memory object
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SIZE);

    // map the shared memory object to memory
    void* ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    auto myMap = static_cast<map<int, set<int>>*>(ptr);

    // update the map
    myMap->insert({1, {1}});
    myMap->at(1).insert(2);
    myMap->at(1).insert(3);

    // spawn a child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        // child process updates the shared memory object
        myMap->insert({2, {2}});
        myMap->at(2).insert(3);
        myMap->at(2).insert(4);
        cout << "Child process updated the map:" << endl;
        printMap(myMap);
    }
    else {
        // parent process waits for the child process to finish
        wait(NULL);
        cout << "Parent process updated the map:" << endl;
        printMap(myMap);

        // cleanup the shared memory object
        munmap(ptr, SIZE);
        shm_unlink(name);
    }

    return 0;
}

void printMap(map<int, set<int>>* myMap) {
    for (auto it = myMap->begin(); it != myMap->end(); ++it) {
        cout << it->first << ": ";
        for (auto it_set = it->second.begin(); it_set != it->second.end(); ++it_set) {
            cout << *it_set << " ";
        }
        cout << endl;
    }
}