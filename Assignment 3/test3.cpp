#include <iostream>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

using namespace std;

int main() {
    // Create shared memory segment for adjacency list
    key_t key_adj = ftok("/dev/null", 'A');
    int shmid_adj = shmget(key_adj, sizeof(vector<vector<int>>), IPC_CREAT | 0666);
    if (shmid_adj == -1) {
        cerr << "Failed to create shared memory segment for adjacency list." << endl;
        return 1;
    }

    // Attach shared memory segment for adjacency list
    void* addr_adj = shmat(shmid_adj, NULL, 0);
    if (addr_adj == reinterpret_cast<void*>(-1)) {
        cerr << "Failed to attach shared memory segment for adjacency list." << endl;
        return 1;
    }

    // Create adjacency list
    vector<vector<int>>* adj_list = static_cast<vector<vector<int>>*>(addr_adj);
    int num_nodes = 5;
    adj_list->resize(num_nodes);
    (*adj_list)[0] = {1, 2};
    (*adj_list)[1] = {0, 2, 3};
    (*adj_list)[2] = {0, 1, 3};
    (*adj_list)[3] = {1, 2, 4};
    (*adj_list)[4] = {3};

    // Create shared memory segment for degree map
    key_t key_deg = ftok("/dev/null", 'B');
    int shmid_deg = shmget(key_deg, sizeof(map<int, int>), IPC_CREAT | 0666);
    if (shmid_deg == -1) {
        cerr << "Failed to create shared memory segment for degree map." << endl;
        return 1;
    }

    // Attach shared memory segment for degree map
    void* addr_deg = shmat(shmid_deg, NULL, 0);
    if (addr_deg == reinterpret_cast<void*>(-1)) {
        cerr << "Failed to attach shared memory segment for degree map." << endl;
        return 1;
    }

    // Create degree map
    map<int, int>* degree = static_cast<map<int, int>*>(addr_deg);
    for (int i = 0; i < num_nodes; i++) {
        (*degree)[i] = (*adj_list)[i].size();
    }

    // Print graph and degree
    cout << "Graph:" << endl;
    for (int i = 0; i < num_nodes; i++) {
        cout << i << ": ";
        for (int j : (*adj_list)[i]) {
            cout << j << " ";
        }
        cout << endl;
    }
    cout << "Degree:" << endl;
    for (auto& p : *degree) {
        cout << p.first << ": " << p.second << endl;
    }

    // Detach shared memory segments
    if (shmdt(addr_adj) == -1) {
        cerr << "Failed to detach shared memory segment for adjacency list." << endl;
        return 1;
    }
    if (shmdt(addr_deg) == -1) {
        cerr << "Failed to detach shared memory segment for degree map." << endl;
        return 1;
    }

    // Destroy shared memory segments
    // if (shmctl(shmid_adj, IPC_RMID, NULL) == -1)
    return 0;
}