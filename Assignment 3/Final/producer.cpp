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
struct Compare
{
    bool operator()(pair<int, int> a, pair<int, int> b)
    {
        return a.second < b.second; // compare second elements
    }
};
int main()
{
    srand(time(NULL));
    int shm_id = shmget(200, 2 * 1024 * 1024, 0666);
    if (shm_id < 0)
    {
        cout << "Error: could not create shared memory segment." << endl;
        return 1;
    }
    void *shm_ptr = shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1)
    {
        cout << "Error: could not attach shared memory segment." << endl;
        return 1;
    }
    // cast the pointer to a pointer of Node pointers
    int *shm_int_first = static_cast<int *>(shm_ptr);
    size_t NUM_NODES = (shm_int_first[1]);
    size_t NUM_EDGES = (shm_int_first[0] / 2 - 1);
    map<int, int> degree;
    for (int i = 2; i < shm_int_first[0]; i++)
        degree[shm_int_first[i]]++;
    priority_queue<pair<int, int>, vector<pair<int, int>>, Compare> pq; // priority queue of pairs with custom comparator
    for (auto it : degree)
        pq.push(make_pair(it.first, it.second));
    shmdt(shm_ptr);
    while (1)
    {
        sleep(5);
        void *shm_ptr_p = shmat(shm_id, NULL, 0);
        if (shm_ptr_p == (void *)-1)
        {
            cout << "Error: could not attach shared memory segment." << endl;
            return 1;
        }
        // cast the pointer to a pointer of Node pointers
        int *shm_int = static_cast<int *>(shm_ptr_p);
        if (fork() == 0)
        {
            int numNewNodes = rand() % 21 + 10;
            for (int i = 0; i < numNewNodes; i++)
            {
                int numNewEdge = rand() % 20 + 1;
                vector<pair<int, int>> nodes;
                int offset = shm_int[0];
                for (int cnt = 0; cnt < numNewEdge; cnt++)
                {
                    nodes.push_back(pq.top());
                    shm_int[offset + 2 * cnt] = nodes.back().first;
                    shm_int[offset + 2 * cnt + 1] = shm_int[1] + i;
                    pq.pop();
                }
                offset += 2 * numNewEdge;
                shm_int[0] = offset;
                pq.push(make_pair(shm_int[+ i, numNewEdge));
                for (int i = 0; i < nodes.size(); i++)
                    pq.push(make_pair(nodes[i].first, nodes[i].second + 1));
            }
            NUM_NODES += numNewNodes;
            shm_int[1] += numNewNodes;
            shmdt(shm_ptr_p);
            exit(0);
        }
        while (wait(NULL) > 0)
        {
            ;
        }
        ofstream file("output2.txt");
        if (!file.is_open())
        {
            cerr << "Failed to open output file "
                 << "output2.txt" << endl;
            exit(1);
        }
        file << shm_int[0] << " " << shm_int[1] << " " << shm_int[2] << endl;
        for (int i = 3; i < shm_int[0]; i++)
            file << shm_int[i++] << " " << shm_int[i] << endl;
        file << endl;
        file.close();
        shmdt(shm_ptr_p);
    }
    
    // shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}