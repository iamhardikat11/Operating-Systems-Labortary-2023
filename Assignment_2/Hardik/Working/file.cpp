#include <bits/stdc++.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <wait.h>
#include <sys/file.h>

using namespace std;
#define DIR_LENGTH 1000
#define kBufferSize 100
std::vector<int> get_pids_with_file_open(const std::string &file_path)
{
  std::vector<int> pids;
  std::string cmd = "lsof -t " + file_path;
  std::array<char, kBufferSize> buffer;
  std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe)
    return pids;
  while (!feof(pipe.get()))
  {
    if (fgets(buffer.data(), kBufferSize, pipe.get()) != nullptr)
    {
      int pid;
      std::istringstream(buffer.data()) >> pid;
      cout << "process_pid" << " " << pid << "\n";
      pids.push_back(pid);
    }
  }
  return pids;
}
std::vector<int> get_pids_with_file_lock(const std::string &file_path)
{
  std::vector<int> pids;
  std::string lock_file = file_path + ".lock";
  std::ifstream locks(lock_file);
  if (!locks.is_open())
    return pids;
  std::unordered_set<int> pid_set;
  std::string line;
  while (std::getline(locks, line))
  {
    int pid;
    std::istringstream(line) >> pid;
    cout << "lock_pid" << pid << "\n";
    pid_set.insert(pid);
  }
  std::copy(pid_set.begin(), pid_set.end(), std::back_inserter(pids));
  return pids;
}

void delop(string file_path)
{
  cout << file_path << endl;
  vector<int> pid_open;
  vector<int> pid_flock;
  int x = fork();
  if (x == 0)
  {
    cout << "Child Process\n";
    pid_open = get_pids_with_file_open(file_path);
    pid_flock = get_pids_with_file_lock(file_path);
  }
  wait(NULL);
  cout << "123" << " " << pid_flock.size() << endl;
  for(auto it: pid_open)
    cout << it << " ";
  cout << endl;
  for(auto it: pid_flock)
    cout << it << " ";
  cout << endl;
}
int main()
{
  char *input;
  while (1)
  {
    input = readline("$ "); // read input from the terminal
    if (input == NULL)      // exit if EOF is reached
      break;
    add_history(input); // add input to the history
    if (input[0] == 'c' && input[1] == 'd')
    {

      int result = chdir((input + 3));
      if (result != 0)
        std::cerr << "Error: failed to change directory to " << (input + 3) << std::endl;
    }
    else if (!strcmp(input, "pwd"))
    {
      char *pwd = (char *)malloc(DIR_LENGTH * sizeof(char));
      getcwd(pwd, DIR_LENGTH);
      printf("%s\n", pwd);
    }
    else if (!strcmp(input, "exit"))
    {
      exit(0);
    }
    else if (strlen(input) > 7 && input[0] == 'd' && input[1] == 'e' && input[2] == 'l' && input[3] == 'e' && input[4] == 'p')
    {
      string file_path = "";
      for (int i = 6; i < strlen(input); i++)
        file_path.push_back(input[i]);
      delop(file_path);
    }
    else
    {
      cout << "INVALID COMMAND\n"
           << endl;
    }
    free(input);
  }
  return 0;
}
