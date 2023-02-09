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
// string 
void traverseProcessTree(int pid, int indent, bool suggest, int depth)
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
    if (!file) 
    { 
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
    if (time > 10 || (children >= 5 && children < 10))
    {
      cout << string(indent, ' ') << "SUSPICIOUS PROCESS!" << endl;
    }
  }

  // Recursively traverse the process tree
  int parentPID = atoi(details["PPid"].c_str());
  if (parentPID > 0)
  {
    traverseProcessTree(parentPID, indent, suggest, depth++);
  }
}

int main()
{
  // Read the input as a string
  string input;
  cout << "Enter the input in the format: [PID] [-suggest]" << endl;
  getline(cin, input);

  // Split the input into two parts: the PID and the flag
  int pos = input.find(" ");
  if (pos == string::npos)
  {
    cout << "Incorrect input format for sb command" << endl;
    exit(1);
  }
  int pid = stoi(input.substr(0, pos));
  string flag = input.substr(pos + 1);

  // Check if the suggest flag is set
  bool suggest = false;
  if (flag == "-suggest")
  {
    suggest = true;
  }
  // Traverse the process tree
  traverseProcessTree(pid, 0, suggest, 0);

  return 0;
}
