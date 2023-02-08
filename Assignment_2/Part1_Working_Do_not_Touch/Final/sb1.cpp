#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

// Function to read information about a process from the /proc file system
map<string, string> getProcessDetails(int pid)
{
  map<string, string> details;

  // Open the status file for the process
  string filename = "/proc/" + to_string(pid) + "/status";
  ifstream statusFile(filename.c_str());
  if (!statusFile.is_open())
  {
    cout << "Error opening file: " << filename << endl;
    exit(1);
  }

  // Read the details from the status file
  string line;
  while (getline(statusFile, line))
  {
    int pos = line.find(':');
    if (pos != string::npos)
    {
      string key = line.substr(0, pos);
      string value = line.substr(pos + 1);
      details[key] = value;
    }
  }
  return details;
}

// Function to traverse the process tree
void traverseProcessTree(int pid, int indent, bool suggest)
{
  // Read the details of the process
  map<string, string> details = getProcessDetails(pid);

  // Print the process ID and name
  cout << string(indent, ' ') << "Process ID: " << pid << endl;
  cout << string(indent, ' ') << "Process Name: " << details["Name"] << endl;
  // Check if the suggest flag is set
  if (suggest)
  {
    // Get the time spent by the process
    long time = atoi(details["Uptime"].c_str());

    // Get the number of child processes
    // DIR *dir = opendir(("/proc/" + to_string(pid) + "/task/" +  to_string(pid) + "/children").c_str());
    string s = "/proc/" + to_string(pid) + "/task/" +  to_string(pid) + "/children";
    ifstream file(s); 
    if (!file) { 
      cout << "File not found!" << endl; 
      return; 
    }  
    string word; 
    int children = 0; 
    while (file >> word) 
    { 
      children++; 
    }  
    file.close();
    cout << time << " " << children << endl;
    if (time > 10 || children >= 5)
    {
      cout << string(indent, ' ') << "SUSPICIOUS PROCESS!" << endl;
    }
  }

  // Recursively traverse the process tree
  int parentPID = atoi(details["PPid"].c_str());
  if (parentPID > 0)
  {
    traverseProcessTree(parentPID, indent + 2, suggest);
  }
}

int main(int argc, char *argv[])
{
  // Check if the correct number of arguments was provided
  if (argc != 2 && argc != 3)
  {
    cout << "Usage: sb [PID] [-suggest]" << endl;
    exit(1);
  }

  // Get the process ID
  int pid = atoi(argv[1]);

  // Check if the suggest flag is set
  bool suggest = false;
  if (argc == 3 && string(argv[2]) == "-suggest")
  {
    suggest = true;
  }

  // Traverse the process tree
  traverseProcessTree(pid, 0, suggest);

  return 0;
}