// #include <iostream>
// #include <unistd.h>
// #include <sys/types.h>
// #include <pwd.h>
// #include <vector>
// #include <map>
// #include <cstring>
// #include <algorithm>
// #include <ctime>
// #include <signal.h>
// using namespace std;

// map<int, vector<int>> children;       // Store the parent-child relationships of processes
// vector<pair<int, clock_t>> processes; // Store the process id and start time for each process

// void display_processes(int pid)
// {
//     cout << "Process ID: " << pid << endl;
//     for (int i = 0; i < children[pid].size(); i++)
//     {
//         cout << "Child: " << children[pid][i] << endl;
//         display_processes(children[pid][i]);
//     }
// }

// int main(int argc, char *argv[])
// {
//     if (argc < 2)
//     {
//         cout << "ERROR: Please provide a process id." << endl;
//         return 1;
//     }
//     int pid = atoi(argv[1]);
//     bool suggest = false;
//     if (argc > 2 && strcmp(argv[2], "-suggest") == 0)
//     {
//         suggest = true;
//     }

//     // Start a child process to detect malwares
//     // pid_t child_pid = fork();
//     // if (child_pid == 0)
//     {
//         // Child process
//         // Get the parent and grandparent of the given process id
//         int parent = getppid();
//         int grandparent = getppid();
//         while (grandparent != 0)
//         {
//             children[grandparent].push_back(parent);
//             parent = grandparent;
//             grandparent = getppid();
//         }

//         // Display the parent-child relationships of the given process id
//         cout << "Parent-Child Relationships:" << endl;
//         display_processes(pid);

//         if (suggest)
//         {
//             // Find the process id with the most child processes or the longest time spent
//             int suspicious_pid = 0;
//             int max_children = 0;
//             clock_t max_time = 0;
//             for (int i = 0; i < processes.size(); i++)
//             {
//                 int curr_pid = processes[i].first;
//                 int curr_children = children[curr_pid].size();
//                 clock_t curr_time = clock() - processes[i].second;
//                 if (curr_children > max_children || curr_time > max_time)
//                 {
//                     suspicious_pid = curr_pid;
//                     max_children = curr_children;
//                     max_time = curr_time;
//                 }
//             }
//             // Display the suspected process id
//             cout << "Suspicious Process ID: " << suspicious_pid << endl;
//         }
//     }
//     return 0;    
// }
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/inotify.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <vector>
#include <fnmatch.h>
#include <algorithm>
#include <glob.h>
#include <cstring>
#include <sstream>
#include <fstream>
#include <string>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int get_parent_pid(int pid)
{
    std::stringstream ss;
    ss << "/proc/" << pid << "/status";
    std::ifstream file(ss.str());
    std::string line;

    while (std::getline(file, line))
    {
        if (line.substr(0, 5) == "PPid:")
        {
            std::stringstream ppid_ss(line.substr(5));
            int ppid;
            ppid_ss >> ppid;
            return ppid;
        }
    }

    return -1;
}

void kill_process_tree(int pid)
{
    std::stringstream ss;
    ss << "/proc/" << pid << "/task";
    std::vector<int> pids;
    DIR *dir = opendir(ss.str().c_str());
    struct dirent *entry;

    while ((entry = readdir(dir)) != nullptr)
    {
        int child_pid;
        std::stringstream pid_ss(entry->d_name);
        pid_ss >> child_pid;
        if (child_pid != 0)
        {
            pids.push_back(child_pid);
        }
    }

    closedir(dir);

    for (int p : pids)
    {
        kill_process_tree(p);
    }

    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: detect_malware <pid>" << std::endl;
        return 1;
    }
    int pid;
    std::stringstream pid_ss(argv[1]);
    pid_ss >> pid;
    int ppid = get_parent_pid(pid);
    while (ppid != -1)
    {
        std::cout << "Process ID: " << pid << std::endl;
        std::cout << "Parent Process ID: " << ppid << std::endl;
        std::cout << std::endl;

        pid = ppid;
        ppid = get_parent_pid(pid);
    }
    kill_process_tree(pid);
    return 0;
}