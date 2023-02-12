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

using namespace std;

#define CMD_LEN 1024
#define IND_CMD_LEN 128
#define BUF_LEN 128
#define EVENT_BUF_LEN 1024
#define MAX_FILENAME_LENGTH 128
#define STD_INPUT 0
#define STD_OUTPUT 1
#define HISTFILESIZE 10000
#define HISTSIZE 1000

char *prompt = (char *)"\n$: ";
char *prompt1 = (char *)"$: ";
/**
 * Terminos has been used to take input character
 * by character with getchar in such a way that if tab
 * or Ctrl+R is pressed then it can be detected directly
 * without the need of pressing enter and processing the input
 */
struct termios saved_attributes;
// function used to reset input to normal mode
void reset_input_mode(void)
{
  saved_attributes.c_lflag |= (ICANON | ECHO);
  saved_attributes.c_cc[VMIN] = 0;
  saved_attributes.c_cc[VTIME] = 1;
  tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}
// command to set input to character by character input mode
void set_input_mode(void)
{
  struct termios tattr;
  tcgetattr(STDIN_FILENO, &saved_attributes);
  atexit(reset_input_mode);
  tcgetattr(STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON | ECHO);
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
}

// global pids where the commands run,
// Kept here so that they can be signalled
// to continue when Ctrl Z is pressed
pid_t *pids;
int pipeCount, watchCmdCnt;

// some flags to switch code to background
// or stop the multiWatch code
static volatile int runInBackgrnd = 0;
static volatile int stopWatch = 0;

// Inotify File Descriptor that watches over temporary files
// during multiwatch. Kept Global so that it can be closed
// when Ctrl+C is pressed.
int inotifyFd;
std::vector<std::string> list_files(const std::string &dir, const std::string &pattern)
{
  std::vector<std::string> files;
  DIR *dp;
  struct dirent *entry;
  if ((dp = opendir(dir.c_str())) == NULL)
  {
    std::cerr << "Cannot open directory: " << dir << std::endl;
    return files;
  }
  while ((entry = readdir(dp)) != NULL)
  {
    std::string file_name = entry->d_name;
    if (fnmatch(pattern.c_str(), file_name.c_str(), FNM_PATHNAME | FNM_PERIOD) == 0)
      files.push_back(file_name);
  }
  closedir(dp);
  return files;
}

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
  // printf("")
  // Check if the suggest flag is set
  if (suggest)
  {
    // Get the time spent by the process
    long time = atoi(details["Uptime"].c_str());

    // Get the number of child processes
    // DIR *dir = opendir(("/proc/" + to_string(pid) + "/task/" +  to_string(pid) + "/children").c_str());
    string s = "/proc/" + to_string(pid) + "/task/" + to_string(pid) + "/children";
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

void expandWildcards(const std::string &arg, std::vector<std::string> &args)
{
  glob_t globBuffer;
  memset(&globBuffer, 0, sizeof(globBuffer));
  int globResult = glob(arg.c_str(), GLOB_TILDE, NULL, &globBuffer);
  if (globResult == 0)
  {
    for (size_t i = 0; i < globBuffer.gl_pathc; ++i)
      args.push_back(globBuffer.gl_pathv[i]);
  }
  globfree(&globBuffer);
}

// This function search file in the directory .
// The function uses trie data structure to
// store the file names in the folder


/**
 * @brief
 * This is a helper function of searchInDirectory Function
 * It helps to traverse on the trie data structure
 * @param isEnd : Map to store the data related to the trie
 * isEnd[i].first = number of file end points in the subtree of node i
 * isEnd[i].second = 1 if the  a file ends here else 0
 * @param fileTrie a trie data structure to store the file names in the directory
 * @param index : the node value
 * @param files : a pointer pointing to a array of char* to store the file names found till now
 * @param filesPushed : a integer that represents the number of files pushed in the 'files' array
 * @param fileName : file name till current node
 * @param fileIndex : index where the next character should be placed in filename
 */
void dfs(std::map<int, std::pair<int, int>> &isEnd,
         std::vector<std::map<char, int>> &fileTrie, int index, char **files,
         int &filesPushed, char *fileName, int fileIndex)
{
  if (isEnd[index].second == 1)
  {
    fileName[fileIndex] = '\0';
    files[filesPushed] = (char *)malloc(sizeof(char) * MAX_FILENAME_LENGTH);
    strcpy(files[filesPushed], fileName);
    files[filesPushed][(int)strlen(fileName)] = '\0';
    filesPushed++;
  }
  for (auto &x : fileTrie[index])
  {
    fileName[fileIndex] = x.first;
    dfs(isEnd, fileTrie, x.second, files, filesPushed, fileName, fileIndex + 1);
    fileName[fileIndex] = '\0';
  }
}

/**
 * @brief
 * The function gets executed when tab is pressed
 * @param cmd: char pointer: command when the tab was pressed
 * @param pos : integer : index where new character should be placed in the array
 * @param isFound : variable to check if any file exists or not
 * @return char*: updated command:
 *               = NULL if no file is found
 *               = command after completion of the file name
 */

class shell_history
{
public:
  int index;
  char **commands;
  int maxSize;
  /**
   * @brief Construct a new shell history object
   *
   */
  shell_history()
  {
    this->index = 0;
    this->maxSize = HISTFILESIZE;
    this->commands = (char **)malloc(sizeof(char *) * HISTFILESIZE);
    int fdRead = open(".history.txt", O_CREAT | O_RDONLY, 0666);
    FILE *file = fdopen(fdRead, "r");
    char *command = NULL;
    size_t len = 0;
    while (getline(&command, &len, file) != -1)
    {
      commands[index] = (char *)malloc(sizeof(char) * strlen(command));
      strcpy(commands[index], command);
      commands[index][(int)strlen(commands[index]) - 1] = '\0';
      index++;
    }
    fclose(file);
  }

  /**
   * @brief
   * The function prints the top 1000 commands in the history
   *
   */
  void print()
  {
    int ind = max(0, index - HISTSIZE);
    for (int i = index - ind - 1, j = 1; i >= 0; i--, j++)
      fprintf(stdout, "%d) %s\n", j, commands[i + ind]);
  }

  /**
   * @brief
   * The command is added in the shell_history
   * @param command
   */
  void push(char *command)
  {
    if (this->index == this->maxSize)
    {
      this->maxSize = this->maxSize + HISTFILESIZE;
      this->commands = (char **)realloc(this->commands, sizeof(char *) * (this->maxSize));
    }
    this->commands[this->index] = (char *)malloc(sizeof(char) * CMD_LEN);
    strcpy(this->commands[this->index], command);
    this->commands[this->index][(int)strlen(command)] = '\0';
    this->index++;
  }
  void pop()
  {
    if (index == 0)
      return;
    // decrement the size of the array
    this->index--;
    // allocate a new array with one less element
    this->commands = (char **)realloc(this->commands, this->index * sizeof(char *));
    // return the new array
  }
  /**
   * @brief
   * THis function write the recent 10000 commands
   * in the file.
   */
  void updateFile()
  {
    FILE *file = fopen(".history.txt", "w");
    int ind = max(0, index - HISTFILESIZE);
    for (int i = ind; i < index; i++)
    {
      fprintf(file, "%s", this->commands[i]);
      fprintf(file, "\n");
    }
    fclose(file);
  }
};

/**
 * @brief Handler to perform a specific function when CTRL+C is
 * pressed.
 *
 * @param dum integer variable that carries the integer value of
 * the signal received. Here it is SIGINT as this is with for a SIGINT Handler
 */
void ctrlCHandler(int dum)
{
  stopWatch = 1;
  fprintf(stdout, " Ctrl C Detected.\n");
  if (inotifyFd != -1)
    close(inotifyFd);
  return;
}

/**
 * @brief Handler to perform a specific function when CTRL+C is
 * pressed.
 *
 * @param dum integer variable that carries the integer value of
 * the signal received. Here it is SIGTSTP as this is with for a SIGTSTP Handler
 */
void ctrlZHandler(int dum)
{
  // try and catch
  fprintf(stdout, " Ctrl Z Detected.\n");
  int x = 0;
  for (int i = 0; i < pipeCount + 1 && i < sizeof(pids) / sizeof(pid_t); i++)
  {
    try
    {
      if (kill(pids[i], SIGCONT))
        ;
      else
        throw x;
    }
    catch (int x)
    {
      fprintf(stdout, "No More Processes to Kill\n");
    }
  }
  runInBackgrnd = 1;
}

/**
 * @brief Split the command by spaces into a number of tokens to return
 * a NULL terminated array with these tokens. Also redirects the input and output
 * to specific file descriptors.
 *
 * @param cmd The complete command with various terms with spaces
 * @param ipFile File Descriptor where input should be redirected. Modified if there
 *               is < token in the command
 * @param opFile File Descriptor where output should be redirected. Modified if there
 *               is > token in the command
 * @return char** NULL terminated character array with command terms
 */
char **splitCommand(char *cmd, int &ipFile, int &opFile)
{
  char cmd_copy[CMD_LEN] = {0};
  strcpy(cmd_copy, cmd);
  char **cmds;

  char *p;
  p = strtok(cmd, " ");
  int cnt = 0;
  while (p)
  {
    p = strtok(NULL, " ");
    cnt++;
  }
  cmds = (char **)malloc((cnt + 1) * sizeof(char *));

  p = strtok(cmd_copy, " ");
  int id = 0;

  int isRedirected = 0;

  while (p)
  {
    if (strcmp(p, "<") == 0)
    {
      isRedirected = 1;
      p = strtok(NULL, " ");
      if (p == NULL)
      {
        fprintf(stderr, "ERROR. No file given after <. Exitting.\n");
        exit(0);
      }
      int fd = open(p, O_RDONLY);
      if (fd < 0)
      {
        fprintf(stderr, "Error in opening input file. Exitting.\n");
        exit(0);
      }
      ipFile = fd;
      p = strtok(NULL, " ");
    }
    else if (strcmp(p, ">") == 0)
    {
      isRedirected = 1;
      p = strtok(NULL, " ");
      if (p == NULL)
      {
        fprintf(stderr, "ERROR. No file given after >. Exitting.\n");
        exit(0);
      }
      int fd = open(p, O_CREAT | O_WRONLY, 0666);
      if (fd < 0)
      {
        fprintf(stderr, "Error in opening ouput file. Exitting.\n");
        exit(0);
      }
      opFile = fd;
      p = strtok(NULL, " ");
    }
    else
    {
      if (isRedirected)
      {
        fprintf(stderr,
                "Syntax Error. Cannot have command after input or output "
                "redirection.\n");
        exit(0);
      }
      cmds[id] = (char *)malloc((strlen(p) + 1) * sizeof(char));
      strcpy(cmds[id], p);
      cmds[id][strlen(p)] = '\0';
      id++;
      p = strtok(NULL, " ");
    }
  }

  if (ipFile != STD_INPUT)
    dup2(ipFile, STD_INPUT);
  if (opFile != STD_OUTPUT)
  {
    dup2(opFile, STD_OUTPUT);
  }
  cmds[id] = NULL;

  return cmds;
}

void trim(string &s)
{
  while (s.length() && s.back() == ' ')
    s.pop_back();
  int i = 0;
  while (i < (int)s.length() && s[i] == ' ')
    i++;
  s = s.substr(i);
}

bool isChar(char ch)
{
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

/**
 * @brief Function to execute piped or unpiped commands by tokenising the command
 * and then running it using execvp in a child process. Handles piped commands too
 * by creating different child processes for each command and pipes for inter-command
 * data transfer.
 *
 * @param cmd The entire line of input
 * @param isBackGrnd Flag to signify whether the processes are to be run in the background or not.
 *                  If yes then parent process does not wait for child to complete.
 */
void executeCommand(char *cmd, int isBackGrnd)
{
  int ipFile = STD_INPUT, opFile = STD_OUTPUT;
  fprintf(stdout, "\n");

  pipeCount = 0;
  for (int i = 0; cmd[i] != '\0'; i++)
  {
    if (cmd[i] == '|')
      pipeCount++;
  }
  int pipes[pipeCount + 1][2];
  pid_t wpid[pipeCount + 1];
  int status[pipeCount + 1];
  for (int i = 0; i < pipeCount; i++)
  {
    if (pipe(pipes[i]) < 0)
    {
      fprintf(stderr, "ERROR in creating pipes >. Exitting.\n");
      exit(0);
    }
  }

  pids = (pid_t *)malloc((pipeCount + 1) * sizeof(pid_t));
  char *command;
  char **cmds;
  char **commandsPipeSep = (char **)(malloc((pipeCount + 2) * sizeof(char *)));
  for (int i = 0; i < pipeCount + 1; i++)
  {
    if (i == 0)
    {
      command = strtok(cmd, "|");
    }
    else
      command = strtok(NULL, "|");
    commandsPipeSep[i] = (char *)(malloc((strlen(command) + 1) * sizeof(char)));
    strcpy(commandsPipeSep[i], command);
    commandsPipeSep[i][strlen(command)] = '\0';
  }
  commandsPipeSep[pipeCount + 1] = NULL;
  for (int i = 0; i < pipeCount + 1; i++)
  {
    pids[i] = fork();
    if (pids[i] == -1)
    {
      fprintf(stderr, "ERROR in forking child >. Exitting.\n");
      exit(0);
    }
    if (pids[i] == 0)
    {
      for (int j = 0; j < pipeCount; j++)
      {
        if (i != j)
          close(pipes[j][1]);
        if (i - 1 != j)
          close(pipes[j][0]);
      }
      if (i == 0)
      {
        ipFile = STD_INPUT;
      }
      else
        ipFile = pipes[i - 1][0];
      if (i == pipeCount)
      {
        opFile = STD_OUTPUT;
      }
      else
      {
        opFile = pipes[i][1];
      }
      cmds = splitCommand(commandsPipeSep[i], ipFile, opFile);
      char **tem = cmds;

      if (i < pipeCount)
        close(pipes[i][1]);
      if (i > 0)
        close(pipes[i - 1][0]);
      if (execvp(cmds[0], cmds) < 0)
      {
        fprintf(stderr, "\nERROR. Could not execute program %s.\n", cmds[0]);
        exit(0);
      }
      return;
    }
  }
  for (int i = 0; i < pipeCount; i++)
  {
    close(pipes[i][0]);
    close(pipes[i][1]);
  }
  if (!isBackGrnd)
  {
    do
    {
      int done = 0;
      for (int i = 0; i < pipeCount + 1; i++)
        wpid[i] = waitpid(pids[i], &status[i], WUNTRACED | WCONTINUED);
      for (int i = 0; i < pipeCount + 1; i++)
      {
        if (WIFEXITED(status[i]) || WIFSIGNALED(status[i]) ||
            WIFCONTINUED(status[i]))
        {
          done = 1;
          break;
        }
      }
      if (done || runInBackgrnd)
      {
        break;
      }
    } while (true);
  }
}
struct Data
{
  string COMMAND;
  string PID;
  string USER;
  string FD;
  string TYPE;
  string DEVICE;
  string SIZE_OFF;
  string NODE;
  string NAME;
};

void parseFile(const string &fileName, vector<string> &pids, vector<string> &pid_lock)
{
  ifstream file(fileName);
  // vector<string> pids;
  string line;
  int lineNum = 0;
  while (getline(file, line))
  {
    lineNum++;
    if (lineNum == 1)
      continue; // skip the first line (header)
    istringstream ss(line);
    string token;
    int tokenNum = 0;
    vector<string> total;
    while (getline(ss, token, ' '))
    {
      tokenNum++;
      if (token.size() != 0)
      {
        total.push_back(token);
        if (tokenNum == 2)
          pids.push_back(token);
      }
    }
    for (auto it : total)
    {
      cout << it << " ";
    }
    cout << endl;
    if (total[3].find("W") != string::npos)
    {
      pid_lock.push_back(total[1]);
    }
  }
  cout << pid_lock.size() << endl;
  // return pids;
}


char **get_arg0(char *cmd)
{
  char cmd_copy[1000] = {0};
  strcpy(cmd_copy, cmd);
  char **arg;

  char *p = strtok(cmd, " ");
  int cnt = 0;
  while (p)
  {
    p = strtok(NULL, " ");
    cnt++;
  }
  arg = (char **)calloc((cnt + 1), sizeof(char *));
  p = strtok(cmd_copy, " ");
  int i = 0;
  for (; p; i++)
  {
    arg[i] = (char *)calloc((strlen(p) + 1), sizeof(char));
    strcpy(arg[i], p);
    arg[i][strlen(p)] = '\0';
    p = strtok(NULL, " ");
  }
  arg[i] = NULL;
  return arg;
}
// void runExtCmd0(char *usr_cmd, char *file)
// {

// }
void delep(char *cmd)
{
  vector<string> data_open, data_lock;
  char *ch = (char *)malloc((CMD_LEN + 1000) * sizeof(char));
  char *ch1 = (char *)malloc(100 * sizeof(char));
  pid_t pid = fork();
  if (pid == 0)
  {
    memset(ch, '\0', (CMD_LEN + 1000));
    memset(ch1, '\0', (100));
    strcpy(ch1, "lsof ");

    char *ch2 = (char *)malloc(100 * sizeof(char));
    memset(ch2, '\0', (100));

    strcpy(ch2, "tmpfile.csv");
    strcat(ch, ch1);
    strcat(ch, cmd);
    cout << 1 << endl;
    // runExtCmd0(ch, ch2);
    cout << ch << " " << ch2 << endl;
    char **arg = get_arg0(ch);
    int fd = open(ch2, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
      perror("open");
    if (dup2(fd, STDOUT_FILENO) < 0)
      perror("dup2");
    if (execvp(arg[0], arg) < 0)
      fprintf(stderr, "\nERROR. Could not execute program.\n");
    cout << 1 << endl;
    // executeCommand(ch, 0);
    // lsof cmd > tmpfile.csv
  }
  else
  {
    // wait(NULL);
    int status;
    waitpid(pid, &status, 0);
    parseFile("tmpfile.csv", data_open, data_lock);
    fprintf(stdout, "PID's that have the file open\n\n");
    for (auto pid_open : data_open)
      cout << pid_open << "   ";
    cout << endl;
    fprintf(stdout, "PID's that have the file lock\n\n");
    for (auto pid_lock : data_lock)
      cout << pid_lock << "   ";
    cout << endl;
    fprintf(stdout, "\n[?] Enter YES or NO to Kill the Processes:- ");
    string t;
    cin >> t;
    if (t.compare("YES") == 0)
    {
      for (auto pid : data_open)
      {
        if (kill(stoi(pid), SIGKILL) < 0)
          perror("Error with KILL.");
      }
      if (remove(cmd) > 0)
      {
        perror("Error: Removing File.");
      }
    }
    cout << endl;
    if (remove((char *)"tmpfile.csv"))
      ;
    free(ch);
  }
}
/**
 * @brief Function to read input command from command line
 *
 * @param isBackGrnd Flag passed by referrence to check if command has & present in
 *                  in the end and needs to be run in background
 * @param needExecution
 * @param shellHistory Object of the shellHistory class that maintains the history of commands run till now.
 * @return char* The entire command line input
 */
char *readLine(int &isBackGrnd, int &needExecution, shell_history &shellHistory)
{
  char *cmd = (char *)malloc(CMD_LEN * sizeof(char));
  char ch = 'a';
  int pos = 0;
  int inputModeSet = 0;
  set_input_mode();
  char **history = shellHistory.commands;
  int hist_cur = max(0, shellHistory.index);
  int cnt = 0;
  int flag_hist = 0;
  while (1)
  {
    ch = getchar();
    if ((int)ch == 1)
    {
      flag_hist = 0;
      for (int i = 0; i < pos; i++)
      {
        char *pr = (char *)"\033[1D";
        fputs(pr, stdout);
      }
      pos = 0;
      continue;
    }
    else if ((int)ch == 5)
    {
      flag_hist = 0;
      for (int i = 0; i < strlen(cmd) - pos; i++)
      {
        char *pr = (char *)"\033[1C";
        fputs(pr, stdout);
      }
      pos = strlen(cmd);
      continue;
    }
    else if ((int)ch == 127)
    {
      if (pos == 0)
        continue;
      else
      {
        pos--;
        cmd[pos] = '\0';
        fputs("\b \b", stdout);
      }
    }
    else if ((int)ch == 27)
    {
      char ch1 = getchar();
      if ((int)ch1 == 91)
      {
        char ch2 = getchar();
        if ((int)ch2 == 65)
        {
          for (int i = 0; i < strlen(cmd) + strlen(prompt); i++)
            fputs("\b \b", stdout);
          fputs(prompt1, stdout);
          // Up Arrow Key
          if (hist_cur >= 0 && shellHistory.index)
          {
            hist_cur--;
            if (hist_cur == -1)
              hist_cur = 0;
            cmd = history[hist_cur];
            pos = strlen(cmd);
            cnt++;
          }
          fprintf(stdout, cmd);
        }
        // Down Arrow Key
        else if ((int)ch2 == 66 && shellHistory.index)
        {
          for (int i = 0; i < strlen(cmd) + strlen(prompt); i++)
            fputs("\b \b", stdout);
          fputs(prompt1, stdout);
          if (hist_cur < shellHistory.index)
          {
            hist_cur++;
            if (hist_cur == shellHistory.index)
              hist_cur = shellHistory.index - 1;
            cmd = history[hist_cur];
            pos = strlen(cmd);
            cnt++;
          }
          fprintf(stdout, cmd);
        }
        // Left Arrow Key
        else if ((int)ch2 == 68)
        {
          if (pos > 0)
          {
            pos--;
            char *pr = (char *)"\033[1D";
            fputs(pr, stdout);
          }
        }
        // Right Arrow Key
        else if ((int)ch2 == 67)
        {
          if (pos < strlen(cmd))
          {
            pos++;
            char *pr = (char *)"\033[1C";
            fputs(pr, stdout);
          }
        }
      }
    }
    else if ((ch == EOF || ch == '\n' || pos >= CMD_LEN - 1))
      break;
    else
    {
      cmd[pos++] = ch;
      putchar(ch);
    }
  }
  if (pos == 0)
  {
    needExecution = 0;
    printf("\n");
    return cmd;
  }
  if (cmd[pos - 1] == '&')
    isBackGrnd = 1;
  cmd[pos] = '\0';
  reset_input_mode();
  return cmd;
}

/**
 * @brief Function to execute the cd command used to switch between directories
 *
 * @param cmd Entire command line input
 */
void executeCd(char *cmd)
{
  int ipFile = STD_INPUT, opFile = STD_OUTPUT;
  char **cmds = splitCommand(cmd, ipFile, opFile);
  if (cmds[1] == NULL)
  {
    fprintf(stderr, "ERROR. No argument to cd.\n");
  }
  else
  {
    if (chdir(cmds[1]) != 0)
      fprintf(stderr, "ERROR. Directory not found.\n");
  }
}

/**
 * @brief Function to execute the help command used to get help on using the terminal
 *
 * @param cmd Entire command line input
 */
void executeHelp(char *cmd)
{
  fprintf(stdout,
          "SHELL HELP\nThis is a SHELL designed as a part of assignment 2 of "
          "OS Lab.\n");
  fprintf(stdout, "Type man <command> to know about a command\n");
}

int main()
{
  // Look out for CTRL+C and CTRL+Z press events
  signal(SIGINT, ctrlCHandler);
  signal(SIGTSTP, ctrlZHandler);
  // signal(SIGKILL, ctrl9Handler);
  shell_history shellHistory;

  while (true)
  {
    fprintf(stdout, prompt);

    // initialise variables
    int isBackgrnd = 0;
    int needExecution = 1;
    inotifyFd = -1;
    // get command line input
    char *cmd = readLine(isBackgrnd, needExecution, shellHistory);
    fprintf(stdout, "\n");
    if (!needExecution)
      continue;
    char *cmdCopy = (char *)malloc(sizeof(char) * CMD_LEN);
    strcpy(cmdCopy, cmd);
    if (cmd[(int)strlen(cmd) - 1] == '&')
      cmd[(int)strlen(cmd) - 1] = '\0';
    shellHistory.push(cmdCopy); // add command to history
    if (strcmp("exit", cmd) == 0)
    { // exit from shell if exit is written
      printf("\n");
      shellHistory.updateFile();
      break;
    }
    // if(strlen(cmd)>=5 && cmd[0]=='e' && cmd[1]=='x' && cmd[2]=='i' && cmd[3]=='t')
    // {
    //   string st = "";
    //   if(cmd[4]!=' ') {
    //     fprintf(stdout,"\nbash: %s: INVALID COMMAND\n", cmd);
    //     continue;
    //   }
    //   for(int i=5;i<strlen(cmd);i++)
    //     st.push_back((char)cmd[i]);
    //   trim(st);
    //   if(st.size() == 0)
    //   {
    //     printf("\n");
    //     shellHistory.updateFile();
    //     break;
    //   }
    //   shellHistory.updateFile();
    //   // if(isNumeric(st))
    //   // {

    //   //   cout << "[" << st << "]" << endl;
    //   // }
    //   // else
    //   // {

    //   // }
    //   // With just numeric
    //   // The terminal process "/usr/bin/bash" terminated with exit code: 123.
    //   // The terminal process "/usr/bin/bash" terminated with exit code: 2.
    //   // exit 123 409 [Number with spaces]
    //   //exit
    //   //bash: exit: too many arguments
    //   // first is a number;
    //   break;
    // }
    if (strcmp("history", cmd) == 0)
    {
      // run special command history
      printf("\n");
      shellHistory.print();
      continue;
    }
    if (cmd[0] == 'd' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'e' && cmd[4] == 'p' && strlen(cmd) >= 7)
    {
      char *cmd1 = (char *)malloc(sizeof(char) * CMD_LEN);
      for (int i = 6; i < strlen(cmd); i++)
        cmd1[i - 6] = cmd[i];
      // fprintf(stdout,cmd1);
      // std::cout << file_path << std::endl;
      delep(cmd1);
      continue;
    }
    // initialise character array to extract part of input
    char *dest = (char *)malloc((11) * sizeof(char));
    for (int i = 0; i < 11; i++)
    {
      dest[i] = '\0';
    }
    if (strlen(cmd) > 2)
      strncpy(dest, cmd, 2);

    if (strcmp(dest, "cd") == 0)
    { // run special command cd
      executeCd(cmd);
      continue;
    }
    if (!strcmp(cmd, "pwd"))
    {
      char *pwd = (char *)malloc(1000 * sizeof(char));
      getcwd(pwd, 1000);
      pwd[strlen(pwd)] = '\0';
      fprintf(stdout, pwd);
      continue;
    }
    if (strlen(cmd) >= 4)
      strncpy(dest, cmd, 4);

    if (strcmp(dest, "help") == 0)
    { // run special command help
      executeHelp(cmd);
      continue;
    }
    // cout << cmd << endl;
    bool flag_wildcard = false;
    for (int i = 0; i < strlen(cmd); i++)
    {
      if (cmd[i] == '?' || cmd[i] == '*')
      {
        flag_wildcard = true;
        break;
      }
    }

    if (flag_wildcard)
    {
      string arg;
      vector<string> args;
      stringstream ss(cmd);
      string c = "";
      for (int i = 0; cmd[i] != ' '; i++)
        c.push_back(cmd[i]);
      while (ss >> arg)
        expandWildcards(arg, args);
      cout << "Expanded arguments:" << endl;
      string expanded_arg_concatenated = "";
      if (strstr(cmd, "sort") != nullptr)
      {
        expanded_arg_concatenated += "sort ";
        for (const auto &arg : args)
        {
          expanded_arg_concatenated += arg + " ";
        }
        cout << expanded_arg_concatenated << endl;
        vector<char *> expanded_args;
        int length = expanded_arg_concatenated.size();
        char *w = (char *)malloc((length + 1) * sizeof(char));
        int i = 0;
        for (char ch : expanded_arg_concatenated)
          w[i++] = ch;
        w[length] = '\0';
        expanded_args.push_back(w);
        for (const auto &exp_arg : expanded_args)
        {
          cout << exp_arg << endl;
          executeCommand(exp_arg, isBackgrnd);
        }
        continue;
      }
      if (strstr(cmd, "gedit") != nullptr)
      {
        expanded_arg_concatenated += "gedit ";
        for (const auto &arg : args)
          expanded_arg_concatenated += arg + " ";
        cout << expanded_arg_concatenated << endl;
        vector<char *> expanded_args;
        int length = expanded_arg_concatenated.size();
        char *w = (char *)malloc((length + 1) * sizeof(char));
        int i = 0;
        for (char ch : expanded_arg_concatenated)
          w[i++] = ch;
        w[length] = '\0';
        expanded_args.push_back(w);
        for (const auto &exp_arg : expanded_args)
        {
          cout << exp_arg << endl;
          executeCommand(exp_arg, isBackgrnd);
        }
        continue;
      }
      else
      {
        string first = "";
        for (int i = 0; i < strlen(cmd); i++)
        {
          if (cmd[i] != ' ')
            first.push_back((char)cmd[i]);
          else {
            first.push_back((char)cmd[i]);
            break;
          }
        }
        expanded_arg_concatenated += first;
        for (const auto &arg : args)
          expanded_arg_concatenated += arg + " ";
        cout << expanded_arg_concatenated << endl;
        vector<char *> expanded_args;
        int length = expanded_arg_concatenated.size();
        char *w = (char *)malloc((length + 1) * sizeof(char));
        int i = 0;
        for (char ch : expanded_arg_concatenated)
          w[i++] = ch;
        w[length] = '\0';
        expanded_args.push_back(w);
        for (const auto &exp_arg : expanded_args)
        {
          cout << exp_arg << endl;
          executeCommand(exp_arg, isBackgrnd);
        }
        continue;
      }
    }
    // test delep with wild_card;
    // sb command starts
    if (cmd[0] == 's' && cmd[1] == 'b')
    {

      int pos = strcspn(cmd, " ");
      if (pos == strlen(cmd))
      {
        cout << "Incorrect input format for sb command" << endl;
        continue;
      }
      char *command = (char *)malloc(sizeof(char) * pos + 1);
      strncpy(command, cmd, pos);
      command[pos] = '\0';

      int pid = atoi(cmd + pos + 1);

      pos = strcspn(cmd + pos + 1, " ");
      if (pos == strlen(cmd))
      {
        cout << "Incorrect input format for sb command" << endl;
        continue;
      }
      char *flag = cmd + pos + 1;

      // Check if the suggest flag is set
      bool suggest = false;
      cout << "Flag:-" << flag << endl;
      if (strstr(cmd, "-suggest") != nullptr)
      {
        suggest = true;
      }
      cout << suggest << endl;
      traverseProcessTree(pid, 0, suggest, 0);
      continue;
    }
    // execute every other command
    executeCommand(cmd, isBackgrnd);
    // flush the standard input and output.
    fflush(stdout);
    fflush(stdin);
  }
  return 0;
}