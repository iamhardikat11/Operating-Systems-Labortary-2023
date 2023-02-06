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

// KMP algorithm is implemented to search for pattern
// matching in history

/**
 * @brief
 *  LSP is longest prefix suffix. lps[i] represents the length
 *  of the longest suffix ending at index i that matches with
 *  the prefix of the string
 * @param str1 -> string passed
 * @param M -> size of the string
 * @param lps -> array to store the lps value
 */
void computeLPSArray(char *str1, int M, int *lps)
{
  int len = 0;
  lps[0] = 0;
  int i = 1;
  while (i < M)
  {
    if (str1[i] == str1[len])
    {
      len++;
      lps[i] = len;
      i++;
    }
    else
    {
      if (len != 0)
      {
        len = lps[len - 1];
      }
      else
      {
        lps[i] = 0;
        i++;
      }
    }
  }
}

/**
 * @brief
 * This function returns the length of the longest
 * prefix of the string1 that is in string 2
 *
 * @param str1
 * @param str2
 * @return int -> length of the longest match
 */
int KMPSearch(char *str1, char *str2)
{
  int M = strlen(str1);
  int N = strlen(str2);
  int lps[M];
  computeLPSArray(str1, M, lps);
  int maxx = 0;
  int i = 0, j = 0;
  while (i < N)
  {
    if (str1[j] == str2[i])
    {
      j++;
      i++;
      maxx = max(j, maxx);
    }
    if (j == M)
    {
      return M;
    }
    else if (i < N && str1[j] != str2[i])
    {
      if (j != 0)
        j = lps[j - 1];
      else
        i = i + 1;
    }
  }
  return maxx;
}

// This function search file in the directory .
// The function uses trie data structure to
// store the file names in the folder
/**
 * @brief
 *  The function reads the file names in the directory,
 * store it in fileTrie(a trie data structure) and
 * update isEnd
 * @param isEnd: a map of pair
 * isEnd[i].first = number of file end points in the subtree of node i
 * isEnd[i].second = 1 if the  a file ends here else 0
 * @param fileTrie a trie data structure to store the file names
 */
void searchInDirectory(std::map<int, std::pair<int, int>> &isEnd,
                       std::vector<std::map<char, int>> &fileTrie)
{
  DIR *dir;
  struct dirent *dirp;
  char **files = (char **)malloc(sizeof(char *));
  files[0] = NULL;
  fileTrie.resize(1);
  fileTrie[0] = {};
  dir = opendir(strdup("."));
  int maxSize = 1;
  while ((dirp = readdir(dir)) != NULL)
  {
    if (dirp->d_type == 4)
    {
      if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
      {
        continue;
      }
    }
    else
    {
      if (dirp->d_name[0] == '.')
        continue;
      int siz = strlen(dirp->d_name);
      int index = 0;
      for (int i = 0; i < siz; i++)
      {
        if (fileTrie[index].find(dirp->d_name[i]) == fileTrie[index].end())
        {
          fileTrie.push_back({});
          fileTrie[index][dirp->d_name[i]] = maxSize++;
        }
        index = fileTrie[index][dirp->d_name[i]];
      }
      index = 0;
      for (int i = 0; i < siz; i++)
      {
        if (isEnd.find(index) == isEnd.end())
          isEnd[index] = {0, 0};
        isEnd[index].first++;
        index = fileTrie[index][dirp->d_name[i]];
      }
      if (isEnd.find(index) == isEnd.end())
        isEnd[index] = {0, 0};
      isEnd[index].first++;
      isEnd[index].second = 1;
    }
  }
  closedir(dir);
  return;
}

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
char *tabHandler(char *cmd, int &pos, int &isFound)
{
  std::map<int, std::pair<int, int>> isEnd;
  std::vector<std::map<char, int>> fileTrie;
  searchInDirectory(isEnd, fileTrie);
  int i = pos - 1;
  for (; i >= 0; i--)
  {
    if ((int)cmd[i] == 32)
      break;
  }
  i++;
  int fileNameStart = i;
  int index = 0;
  while (i < pos)
  {
    if (fileTrie[index].find(cmd[i]) == fileTrie[index].end())
    {
      isFound = 0;
      return NULL;
    }
    index = fileTrie[index][cmd[i]];
    i++;
  }
  int noOfFiles = isEnd[index].first;
  char **files = (char **)malloc(sizeof(char *) * (noOfFiles + 1));
  char *fileName = (char *)malloc(sizeof(char) * MAX_FILENAME_LENGTH);
  i = fileNameStart;
  int j = 0;
  while (i < pos)
  {
    fileName[j++] = cmd[i++];
  }
  int filesPushed = 0;
  dfs(isEnd, fileTrie, index, files, filesPushed, fileName, j);
  files[noOfFiles] = NULL;

  isFound = 1;
  cmd = (char *)realloc(cmd, sizeof(char) * CMD_LEN);
  // Only 1 file exists with the given prefix
  if (noOfFiles == 1)
  {
    int length_ = strlen(files[0]);
    for (int i = 0; i < length_; i++)
    {
      cmd[fileNameStart + i] = files[0][i];
    }
    cmd[fileNameStart + length_] = '\0';
    set_input_mode();
    for (int i = pos; i < fileNameStart + length_; i++)
      putchar(cmd[i]);
    reset_input_mode();
    pos = fileNameStart + length_;
    return cmd;
  }
  int k = 0;
  fprintf(stdout, "\n");
  while (files[k] != NULL)
  {
    printf("%d) %s\n", k + 1, files[k]);
    k++;
  }
  int input = -1;
  // Multiple files exist so asking for the index
  while (input <= 0 || input > noOfFiles)
  {
    fprintf(stdout, "Enter the index: ");
    fscanf(stdin, "%d", &input);
  }
  getchar();
  // updating the command array
  int length_ = strlen(files[input - 1]);
  for (int i = 0; i < length_; i++)
  {
    cmd[fileNameStart + i] = files[input - 1][i];
  }
  cmd[fileNameStart + length_] = '\0';
  pos = fileNameStart + length_;
  fprintf(stdout, "\n");
  fprintf(stdout, "$: ");
  // printing the command
  set_input_mode();
  for (int i = 0; i < pos; i++)
    putchar(cmd[i]);
  reset_input_mode();
  return cmd;
}

/**
 * @brief
 * The class stores the history.
 *
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
    {
      fprintf(stdout, "%d) %s\n", j, commands[i + ind]);
    }
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
      this->commands =
          (char **)realloc(this->commands, sizeof(char *) * (this->maxSize));
    }
    this->commands[this->index] = (char *)malloc(sizeof(char) * CMD_LEN);
    strcpy(this->commands[this->index], command);
    this->commands[this->index][(int)strlen(command)] = '\0';
    this->index++;
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

  /**
   * @brief
   * The function gets executed when
   * CTRL + R is pressed
   *
   */
  void search()
  {
    fprintf(stdout, "Enter the search term: ");
    char *cmd = (char *)malloc(sizeof(char) * CMD_LEN);
    char ch;
    int pos = 0;
    while (1)
    {
      ch = getchar();
      if ((ch == EOF || ch == '\n' || pos >= CMD_LEN - 1))
        break;
      else
      {
        cmd[pos++] = ch;
      }
    }
    cmd[pos] = '\0';
    int maxx = 0;
    int maxIndex = -1;
    for (int cind = this->index - 1; cind >= 0; cind--)
    {
      for (int i = 0; i < (int)strlen(cmd); i++)
      {
        char *cmdsubstring = cmd + i;
        int len = KMPSearch(cmdsubstring, this->commands[cind]);
        if (len > maxx)
        {
          maxx = len;
          maxIndex = cind;
        }
      }
    }
    if (maxx == strlen(cmd))
    {
      fprintf(stdout, "%s\n", commands[maxIndex]);
    }
    else if (maxx <= 2)
    {
      fprintf(stdout, "No match for search term in history\n");
    }
    else
    {
      fprintf(stdout, "%s\n", commands[maxIndex]);
    }
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
  fprintf(stdout, " Ctrl Z Detected.\n");
  for (int i = 0; i < pipeCount + 1; i++)
  {
    kill(pids[i], SIGCONT);
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
      {
        wpid[i] = waitpid(pids[i], &status[i], WUNTRACED | WCONTINUED);
      }
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

/**
 * @brief Function to clear all temporary files after MultiWatch is stopped.
 *
 */
// void stopMultiWatch() {
//   for (int i = 0; i < watchCmdCnt; i++) {
//     char *fileName = (char *)malloc(BUF_LEN * sizeof(char));

//     asprintf(&fileName, ".temp.PID%d.txt", i + 1);

//     remove(fileName);
//   }
// }

// /**
//  * @brief Function to run MultiWatch functionality.
//  *
//  * @param cmd The entire command line input
//  */
// void runMultiWatch(char *cmd) {
//   stopWatch = 0;
//   int len = strlen(cmd);

//   char *cmds = (char *)malloc((CMD_LEN) * sizeof(char));

//   strncpy(cmds, cmd + 12, len - 13);

//   cmds[len - 13] = '\0';

//   watchCmdCnt = 1;

//   for (int i = 0; i < strlen(cmds); i++) {
//     if (cmds[i] == ',') {
//       watchCmdCnt++;
//     }
//   }

//   char **indCmds = (char **)malloc(watchCmdCnt * sizeof(char *));

//   for (int i = 0; i < watchCmdCnt; i++) {
//     indCmds[i] = (char *)malloc(IND_CMD_LEN * sizeof(char));
//   }
//   indCmds[0] = strtok(cmds, ",");
//   strncpy(indCmds[0], indCmds[0] + 1, strlen(indCmds[0]) - 2);
//   indCmds[0][strlen(indCmds[0]) - 2] = '\0';
//   for (int i = 1; i < watchCmdCnt; i++) {
//     indCmds[i] = strtok(NULL, ",");
//     strncpy(indCmds[i], indCmds[i] + 1, strlen(indCmds[i]) - 2);
//     indCmds[i][strlen(indCmds[i]) - 2] = '\0';
//   }
//   int fds[watchCmdCnt], readFds[watchCmdCnt];
//   inotifyFd = inotify_init();
//   map<int, int> wd2File;

//   for (int i = 0; i < watchCmdCnt; i++) {
//     char *fileName = (char *)malloc(BUF_LEN * sizeof(char));

//     asprintf(&fileName, ".temp.PID%d.txt", i + 1);
//     fds[i] = open(fileName, O_CREAT | O_WRONLY, 0666);
//     readFds[i] = open(fileName, O_CREAT | O_RDONLY, 0666);

//     int wd = inotify_add_watch(inotifyFd, fileName, IN_MODIFY);

//     wd2File[wd] = i;
//   }

//   pid_t *watch_pids = (pid_t *)malloc(watchCmdCnt * sizeof(pid_t));

//   int pid1 = fork();

//   // execute commands again and again in children processes
//   if (pid1 == 0) {
//     do {
//       for (int i = 0; i < watchCmdCnt; i++) {
//         watch_pids[i] = fork();

//         if (watch_pids[i] < 0) {
//           fprintf(stderr, "Could not create process. Exitting\n");
//           exit(0);
//         } else if (watch_pids[i] == 0) {  // child process
//           // close all other files
//           for (int j = 0; j < watchCmdCnt; j++) {
//             if (j != i) {
//               close(fds[j]);
//             }
//             close(readFds[i]);
//           }

//           int ipFile = STD_INPUT;

//           char **cmds = splitCommand(indCmds[i], ipFile, fds[i]);

//           if (execvp(cmds[0], cmds) < 0) {
//             fprintf(stderr, "ERROR. Could not execute program %s.\n", cmds[0]);
//             exit(0);
//           }
//         }
//       }
//       sleep(1);
//       if (stopWatch) exit(0);
//     } while (true);
//   }

//   // watch for output using inotifyFd
//   while (true) {

//     char buf[EVENT_BUF_LEN]
//         __attribute__((aligned(__alignof__(struct inotify_event))));
//     int len = read(inotifyFd, buf, sizeof(buf));

//     if (len <= 0 || stopWatch) break;

//     int i = 0;

//     while (i < len) {

//       struct inotify_event *event = (struct inotify_event *)&buf[i];

//       if (event->mask & IN_MODIFY) {
//         int wd = event->wd;

//         int fileIndex = wd2File[wd];

//         fprintf(stdout, "\"%s\" , current_time: %lu.\n", indCmds[fileIndex],
//                 (unsigned long)time(NULL));
//         fprintf(stdout, "<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-\n");
//         char read_buf[BUF_LEN + 1];
//         for (int i = 0; i <= BUF_LEN; i++) {
//           read_buf[i] = '\0';
//         }

//         while (read(readFds[fileIndex], read_buf, BUF_LEN) > 0) {
//           fprintf(stdout, "%s", read_buf);
//           for (int i = 0; i <= BUF_LEN; i++) {
//             read_buf[i] = '\0';
//           }
//         }

//         fprintf(stdout, "->->->->->->->->->->->->->->->->->->->->\n\n");
//       }
//       i += sizeof(struct inotify_event) + event->len;
//     }
//   }

//   // clear data after multiWatch
//   stopMultiWatch();
// }

/**
 * @brief Function to read input command from command line
 *
 * @param isBackGrnd Flag passed by referrence to check if command has & present in
 *                  in the end and needs to be run in background
 * @param needExecution
 * @param shellHistory Object of the shellHistory class that maintains the history of commands run till now.
 * @return char* The entire command line input
 */
char *readLine(int &isBackGrnd, int &needExecution,
               shell_history &shellHistory)
{
  char *cmd = (char *)malloc(CMD_LEN * sizeof(char));

  char ch = 'a';
  int pos = 0;
  int inputModeSet = 0;
  set_input_mode();
  while (1)
  {
    ch = getchar();
    if ((int)ch == 127)
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
    else if ((int)ch == 9)
    {
      // autocomplete work
      reset_input_mode();
      int isFound = -1;
      char *cmd2 = tabHandler(cmd, pos, isFound);
      set_input_mode();
      if (isFound != 1)
        continue;
      cmd = cmd2;
    }
    else if ((int)ch == 18)
    {
      while (pos > 0)
      {
        fputs("\b \b", stdout);
        pos--;
      }
      inputModeSet = 0;
      reset_input_mode();
      shellHistory.search();
      needExecution = 0;
      return cmd;
      // history work
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
  {
    isBackGrnd = 1;
  }
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
    {
      fprintf(stderr, "ERROR. Directory not found.\n");
    }
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

  shell_history shellHistory;

  while (true)
  {
    fprintf(stdout, "\n$: ");

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
    {
      cmd[(int)strlen(cmd) - 1] = '\0';
    }

    shellHistory.push(cmdCopy); // add command to history

    if (strcmp("exit", cmd) == 0)
    { // exit from shell if exit is written
      printf("\n");
      shellHistory.updateFile();
      break;
    }

    if (strcmp("history", cmd) == 0)
    { // run special command history
      printf("\n");
      shellHistory.print();
      continue;
    }
    
    // initialise character array to extract part of input
    char *dest = (char *)malloc((11) * sizeof(char));
    for (int i = 0; i < 11; i++)
    {
      dest[i] = '\0';
    }
    if (strlen(cmd) > 10)
      strncpy(dest, cmd, 10);

    // if (strcmp(dest, "multiWatch") == 0)
    // { // run special command muliwatch
    //   runMultiWatch(cmd);
    // }
    // else
    {
      cout << "Hi" << endl;
      if (strlen(cmd) > 2)
        strncpy(dest, cmd, 2);

      if (strcmp(dest, "cd") == 0)
      { // run special command cd
        executeCd(cmd);
        continue;
      }

      if (strlen(cmd) >= 4)
        strncpy(dest, cmd, 4);

      if (strcmp(dest, "help") == 0)
      { // run special command help
        executeHelp(cmd);
        continue;
      }
      printf("%s\n", cmd);
      // execute every other command
      executeCommand(cmd, isBackgrnd);
      printf("1\n");
    }
    // flush the standard input and output.
    fflush(stdout);
    fflush(stdin);
  }
  return 0;
}