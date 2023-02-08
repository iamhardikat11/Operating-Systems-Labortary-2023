#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <vector>
#include <map>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <signal.h>
using namespace std;

map<int, vector<int>> children;       // Store the parent-child relationships of processes
vector<pair<int, clock_t>> processes; // Store the process id and start time for each process

void display_processes(int pid)
{
    cout << "Process ID: " << pid << endl;
    for (int i = 0; i < children[pid].size(); i++)
    {
        cout << "Child: " << children[pid][i] << endl;
        display_processes(children[pid][i]);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "ERROR: Please provide a process id." << endl;
        return 1;
    }
    int pid = atoi(argv[1]);
    bool suggest = false;
    if (argc > 2 && strcmp(argv[2], "-suggest") == 0)
    {
        suggest = true;
    }

    // Start a child process to detect malwares
    // pid_t child_pid = fork();
    // if (child_pid == 0)
    {
        // Child process
        // Get the parent and grandparent of the given process id
        int parent = getppid();
        int grandparent = getppid();
        while (grandparent != 0)
        {
            children[grandparent].push_back(parent);
            parent = grandparent;
            grandparent = getppid();
        }

        // Display the parent-child relationships of the given process id
        cout << "Parent-Child Relationships:" << endl;
        display_processes(pid);

        if (suggest)
        {
            // Find the process id with the most child processes or the longest time spent
            int suspicious_pid = 0;
            int max_children = 0;
            clock_t max_time = 0;
            for (int i = 0; i < processes.size(); i++)
            {
                int curr_pid = processes[i].first;
                int curr_children = children[curr_pid].size();
                clock_t curr_time = clock() - processes[i].second;
                if (curr_children > max_children || curr_time > max_time)
                {
                    suspicious_pid = curr_pid;
                    max_children = curr_children;
                    max_time = curr_time;
                }
            }
            // Display the suspected process id
            cout << "Suspicious Process ID: " << suspicious_pid << endl;
        }
    }
    return 0;    
}