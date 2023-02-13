// No. of Nodes:- 4039
#include <bits/stdc++.h>
#include <sys/shm.h>
using namespace std;

const int MAX_NODES = 4039;

int main()
{
    char* myseg;
    key_t key;
    int shmid;

    key = 300; // some unique_id

    shmid = shmget(key, 250, IPC_CREAT | 0666);
    // start of IPC Segment
    myseg = (char *)shmat(shmid, NULL, 0);

    vector<vector<int>> graph(MAX_NODES); // Creating the graph using vector of vectors

    // Opening the file
    ifstream input("facebook_combined.txt");
    if (!input) {
        cout << "Unable to open the file." << endl;
        return 1;
    }

    int a, b;
    int cnt = 0;
    while (input >> a >> b) {
        graph[a].push_back(b); // Adding the edge from node 'a' to node 'b'
        graph[b].push_back(a); // Adding the edge from node 'b' to node 'a'
        cnt++;
    }
    // Closing the file
    input.close();

    // Printing the graph to check if it is properly stored
    for (int i = 0; i < graph.size(); i++) {
        cout << i << ": ";
        for (int j = 0; j < graph[i].size(); j++) {
            cout << graph[i][j] << " ";
        }
        cout << endl;
    }
    cout << "Edges:- " << cnt << endl;
    shmdt(myseg);
    // end of IPC Segment
    shmctl(shmid, IPC_RMID, NULL);

}