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
#define MAX_NUM_NODES 90000
vector<vector<int>> g(MAX_NUM_NODES);

void print_dist_vec(vector<vector<int>> &dist)
{
    ofstream file("output4.txt");
    if (!file.is_open())
    {
        cerr << "Failed to open output file "
             << "output4.txt" << endl;
        exit(1);
    }
    for (int i = 0; i < dist.size(); i++)
    {
        file << "dist[" << i << "]:";
        for (auto x : dist[i])
            file << x << " ";
        file << endl;
    }
    file.close();
}

// djikstra algorithm to find shortest path from source to all other nodes in a graph with non-negative edge weights using adjacency list representation of graph
vector<int> djikstra(int src, int n)
{
    vector<int> dist(n + 1, INT_MAX);
    dist[src] = 0;

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.push({0, src});

    while (!pq.empty())
    {
        int u = pq.top().second;
        pq.pop();

        for (int v : g[u])
        {
            if (dist[v] > dist[u] + 1)
            {
                dist[v] = dist[u] + 1;
                pq.push({dist[v], v});
            }
        }
    }
    for (int i = 0; i <= n; i++)
        if (dist[i] == INT_MAX)
            dist[i] = -1;
    return dist;
}

void update_distance(vector<int> &dist, int u, int v)
{
    queue<int> q, temp;
    q.push(v);
    dist[v] = dist[u] + 1;
    int val = 2;
    while (!q.empty())
    {
        int x = q.front();
        q.pop();
        for (int y : g[x])
        {
            if (dist[y] == -1)
            {
                dist[y] = dist[u] + val;
                q.push(y);
            }
            else if (dist[y] > dist[u] + val)
            {
                dist[y] = dist[u] + val;
                temp.push(y);
            }
        }
        if (q.empty())
        {
            q = temp;
            temp = queue<int>();
            val++;
        }
    }
}

void add_more_edges(vector<pair<int, int>> &new_edges)
{
    fstream file;
    string word, t, q, filename;
    int u, v;

    new_edges.clear();
    filename = "new_nodes.txt";
    file.open(filename.c_str());
    while (file >> word)
    {
        // cout<<word<<endl;
        u = stoi(word);
        file >> word;
        v = stoi(word);

        g[u].push_back(v);
        g[v].push_back(u);
        new_edges.push_back({u, v});
    }
    file.close();
}

void update_dist_vec(vector<vector<int>> &dist, vector<pair<int, int>> &new_edges)
{
    for (int i = 0; i < dist.size(); i++)
    {
        for (auto x : new_edges)
        {
            if (dist[i][x.first] >= 0 || dist[i][x.second] >= 0)
            {
                int a, b;
                if (dist[i][x.first] < 0)
                {
                    a = x.second;
                    b = x.first;
                }
                else if (dist[i][x.second] < 0)
                {
                    a = x.first;
                    b = x.second;
                }
                else if (dist[i][x.first] >= 0 && ((dist[i][x.first] + 1) < dist[i][x.second]))
                {
                    a = x.first;
                    b = x.second;
                }
                else if (dist[i][x.second] >= 0 && ((dist[i][x.second] + 1) < dist[i][x.first]))
                {
                    a = x.second;
                    b = x.first;
                }
                else
                    continue;

                cout << "i:" << i << " a:" << a << " b:" << b << endl;
                update_distance(dist[i], a, b);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        cout << "Usage: ./main [-optimize]" << endl;
        exit(1);
    }
    if (argc == 2)
    {
        string s = argv[1];
        if (s != "-optimize")
        {
            cout << "Usage: ./main [-optimize]" << endl;
            exit(1);
        }
    }
    bool optimize = false;
    if (argc == 2)
    {
        optimize = true;
    }
    srand(time(NULL));
    int shm_id = shmget(200, 2 * 1024 * 1024, 0666);
    if (shm_id < 0)
    {
        cout << "Error: could not create shared memory segment." << endl;
        return 1;
    }
    if (optimize)
    {
        // Optimised
        g.clear();
        void *shm_ptr = shmat(shm_id, NULL, 0);
        if (shm_ptr == (void *)-1)
        {
            cout << "Error: could not attach shared memory segment." << endl;
            return 1;
        }
        // cast the pointer to a pointer of Node pointers
        int *shm_int_p = static_cast<int *>(shm_ptr);
        size_t NUM_NODES = (shm_int_p[1]);
        size_t NUM_EDGES = ((shm_int_p[0] - 3) / 2);
        for (int i = g.size(); i < shm_int_p[1]; i++)
            g.push_back(vector<int>());
        ofstream file("output3.txt");
        if (!file.is_open())
        {
            cerr << "Failed to open output file "
                 << "output3.txt" << endl;
            exit(1);
        }
        file << shm_int_p[0] << " " << shm_int_p[1] << " " << shm_int_p[2] << endl;
        for (int i = shm_int_p[2]; i < shm_int_p[0]; i += 2)
        {
            g[shm_int_p[i]].push_back(shm_int_p[i + 1]);
            g[shm_int_p[i + 1]].push_back(shm_int_p[i]);
            file << shm_int_p[i] << " " << shm_int_p[i + 1] << endl;
        }
        shm_int_p[2] = shm_int_p[0];
        shmdt(shm_ptr);
        vector<vector<int>> dist(1);
        dist[0] = djikstra(0, MAX_NUM_NODES);
        print_dist_vec(dist);
        file << endl;
        file.close();
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
                size_t NUM_NODES = (shm_int[1]);
                size_t NUM_EDGES = ((shm_int[0] - 3) / 2);
                ofstream file("output3.txt");
                if (!file.is_open())
                {
                    cerr << "Failed to open output file "
                         << "output3.txt" << endl;
                    exit(1);
                }
                file << shm_int[0] << " " << shm_int[1] << " " << shm_int[2] << endl;
                vector<pair<int, int>> new_edges;
                for (int i = shm_int[2]; i < shm_int[0]; i += 2)
                {
                    g[shm_int[i]].push_back(shm_int[i + 1]);
                    g[shm_int[i + 1]].push_back(shm_int[i]);
                    if (i >= shm_int[2])
                        new_edges.push_back({shm_int[i], shm_int[i + 1]});
                }
                for (int i = 0; i < g.size(); i++)
                {
                    file << i << ": ";
                    for (int j = 0; j < g[i].size(); j++)
                    {
                        file << g[i][j] << " ";
                    }
                    file << endl;
                }
                shm_int[2] = shm_int[0] + 1;
                int n = NUM_NODES, m = NUM_EDGES;
                
                shmdt(shm_ptr_p);
                exit(0);
            }
            while (wait(NULL) > 0)
            {
                ;
            }
            shmdt(shm_ptr_p);
        }
    }
    else
    {
        // Unoptimized Version
        g.clear();
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
                size_t NUM_NODES = (shm_int[1]);
                size_t NUM_EDGES = ((shm_int[0] - 3) / 2);
                ofstream file("output3.txt");
                if (!file.is_open())
                {
                    cerr << "Failed to open output file "
                         << "output3.txt" << endl;
                    exit(1);
                }
                file << shm_int[0] << " " << shm_int[1] << " " << shm_int[2] << endl;
                for (int i = 3; i < shm_int[0]; i += 2)
                {
                    g[shm_int[i]].push_back(shm_int[i + 1]);
                    g[shm_int[i + 1]].push_back(shm_int[i]);
                    file << shm_int[i] << " " << shm_int[i + 1] << endl;
                }
                shm_int[2] = shm_int[0] + 1;
                int n = NUM_NODES, m = NUM_EDGES;
                vector<vector<int>> dist(1);
                dist[0] = djikstra(0, n);
                print_dist_vec(dist);
                file << endl;
                file.close();
                shmdt(shm_ptr_p);
                exit(0);
            }
            while (wait(NULL) > 0)
            {
                ;
            }
            shmdt(shm_ptr_p);
        }
    }
    shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}