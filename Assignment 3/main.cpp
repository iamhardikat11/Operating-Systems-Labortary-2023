#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <vector>
#include <utility>
#include <string>
using namespace std;

int main(int argc, char *argv[])
{
    srand((unsigned long int)time(NULL));
    /*
        Start of Main Process
    */
    // Create shared memory
    int shmid_mem;
    key_t key = ftok("shmfile", 69);
    char *shm, *s;
    // Create the segment of size 2MB
    shmid_mem = shmget(key, 2 * 1024 * 1024, 0666 | IPC_CREAT);
    if (shmid_mem < 0)
    {
        perror("shmget");
        exit(1);
    }
    shm = (char *)shmat(shmid_mem, NULL, 0);
    if (shm == (char *)-1)
    {
        perror("shmat");
        shmctl(shmid_mem, IPC_RMID, NULL);
        exit(1);
    }

    // Now we open the file and load the initial graph into the shared memory
    // We also load the number of vertices and edges into the shared memory
    FILE *fp;
    fp = fopen("facebook_combined.txt", "r");
    if (fp == NULL)
    {
        perror("fopen");
        shmctl(shmid_mem, IPC_RMID, NULL);
        exit(1);
    }
    int n = 0;
    vector<pair<int, int> > edges;
    int u, v;
    while (fscanf(fp, "%d %d", &u, &v) != EOF)
    {
        edges.push_back(make_pair(u, v));
        n = max(n, max(u, v));
    }
    fclose(fp);
    // Now we have the edges in the vector
    // We need to load the edges into the shared memory
    // We also need to load the number of vertices and edges into the shared memory
    // We will load the number of vertices and edges in the first 2 integers
    // We will load the edges in the next 2*edges.size() integers
    int *shm_int = (int *)shm;
    shm_int[0] = n + 1;
    shm_int[1] = edges.size();
    for (int i = 0; i < edges.size(); i++)
    {
        shm_int[2 * i + 2] = edges[i].first;
        shm_int[2 * i + 3] = edges[i].second;
    }
    /*
        End of Main Process
    */
    // Creating Producer Process
    if(fork()==0)
    {
        vector<int> nodes(21);
        int x = 10;
        std::generate(nodes.begin(), nodes.end(), [&]{ return  x++; });
        for(int i=0;i<nodes.size();i++) cout << nodes[i] << " ";
        cout << endl;
        int m = nodes[rand()%20];
        cout << m << endl;
        shm_int[0] += m;
        shmdt(shm_int);
        exit(0);
    }
    wait(NULL);
#ifdef DEBUG
    // Write adjacency list to a file
    FILE *out;
    out = fopen("output.txt", "w");
    // fprintf(out, "%d:::\n\n", shm_int[0]);
    // for (int j = 0; j <= n; j++)
    // {
    //     fprintf(out, "%d: ", j);
    //     for (int i = 0; i < edges.size(); i++)
    //     {
    //         if (shm_int[2 * i + 2] == j)
    //         {
    //             fprintf(out, "%d ", shm_int[2 * i + 3]);
    //         }
    //         if (shm_int[2 * i + 3] == j)
    //         {
    //             fprintf(out, "%d ", shm_int[2 * i + 2]);
    //         }
    //     }
    //     fprintf(out, "\n");
    // }

    // Modify the format of the output
    for (int j = 0; j <= n; j++)
    {
        fprintf(out, "%d: ", j);
        for (int i = 0; i < edges.size(); i++)
        {
            if (shm_int[2 * i + 2] == j)
            {
                fprintf(out, "%d ", shm_int[2 * i + 3]);
            }
            if (shm_int[2 * i + 3] == j)
            {
                fprintf(out, "%d ", shm_int[2 * i + 2]);
            }
        }
        fprintf(out, "\n");
    }

    // Compute and print additional information
    // for (int j = 0; j <= n; j++)
    // {
    //     int degree = 0;
    //     for (int i = 0; i < edges.size(); i++)
    //     {
    //         if (shm_int[2 * i + 2] == j || shm_int[2 * i + 3] == j)
    //         {
    //             degree++;
    //         }
    //     }
    //     fprintf(out, "%d: degree = %d\n", j, degree);
    // }

    fclose(out);
#endif
    shmdt(shm_int);
    shmctl(shmid_mem,IPC_RMID, NULL);
    return 0;
}