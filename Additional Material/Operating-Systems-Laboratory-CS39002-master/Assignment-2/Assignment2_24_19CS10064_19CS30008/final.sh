#include <bits/stdc++.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <deque>
#include <map>
#include <dirent.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <signal.h>
#include <sys/signal.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <deque>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <set>
#include <exception>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <map>
#include <map>
#include <vector>
#include <cstring>
#include <sstream>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

#define RUNNING 0
#define STOPPED 1
#define DONE 2

bool ctrlC = 0, ctrlZ = 0, ctrlD = 0; // Indicates whether the user has pressed Ctrl-C, Ctrl-Z, or Ctrl-D
pid_t fgpid = 0;                      // Foreground process group id

#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN "\033[1;36m"
#define COLOR_RESET "\033[0m"

#define DIR_LENGTH 1000

#define CTRL_CZ -1
#define CTRL_D 4
#define CTRL_R 18
#define BACKSPACE 127
#define TAB 9
#define ENTER 10

class ShellException : public exception
{
protected:
    const string message;

public:
    ShellException(const string &);
    virtual const char *what() const throw();
};

ShellException::ShellException(const string &msg) : message(msg) {}

const char *ShellException::what() const throw()
{
    return message.c_str();
}

class Command
{
public:
    string cmd;                     // The command as a string
    vector<string> args;            // List of arguments in the command
    int fd_in, fd_out;              // Input and output file descriptors
    string input_file, output_file; // Input and output filenames
    pid_t pid;                      // Process ID of the command when it executes

    Command(const string &cmd) : cmd(cmd), fd_in(STDIN_FILENO), fd_out(STDOUT_FILENO), input_file(""), output_file("") {}
    ~Command()
    {
        if (fd_in != STDIN_FILENO)
        {
            close(fd_in);
        }
        if (fd_out != STDOUT_FILENO)
        {
            close(fd_out);
        }
    }

    void parse()
    {
        vector<string> tokens;
        string temp = "";
        for (size_t i = 0; i < cmd.length(); i++)
        {
            if (cmd[i] == '\\')
            { // Escape character
                i++;
                if (i != cmd.length())
                {
                    temp += cmd[i];
                }
                else
                {
                    throw ShellException("Unable to parse '\\'");
                }
                continue;
            }
            if (cmd[i] == '>' || cmd[i] == '<' || cmd[i] == '&')
            {
                if (temp.size() > 0)
                {
                    tokens.push_back(temp);
                    temp = "";
                }
                tokens.push_back(string(1, cmd[i]));
            }
            else if (cmd[i] == '"')
            {
                i++;
                while (i < cmd.length() && (cmd[i] != '"' || (cmd[i] == '"' && cmd[i - 1] == '\\')))
                {
                    temp += cmd[i++];
                }
                if (i == cmd.length())
                {
                    throw ShellException("Unable to parse '\"'");
                }
            }
            else if (cmd[i] == ' ')
            {
                if (temp.size() > 0)
                {
                    tokens.push_back(temp);
                    temp = "";
                }
            }
            else
            {
                temp += cmd[i];
            }
        }
        if (temp.size() > 0)
        {
            tokens.push_back(temp);
        }

        for (size_t i = 0; i < tokens.size(); i++)
        {
            if (tokens[i] == "<")
            { // Input redirection
                i++;
                if (i == tokens.size() || tokens[i] == ">" || tokens[i] == "<" || tokens[i] == "&")
                {
                    throw ShellException("Unable to parse after '<'");
                }
                else
                {
                    input_file = tokens[i];
                }
            }
            else if (tokens[i] == ">")
            { // Output redirection
                i++;
                if (i == tokens.size() || tokens[i] == ">" || tokens[i] == "<" || tokens[i] == "&")
                {
                    throw ShellException("Unable to parse after '>'");
                }
                else
                {
                    output_file = tokens[i];
                }
            }
            else if (tokens[i] == "&")
            { // & is already parsed at the pipeline parsing stage
                throw ShellException("Unable to parse after '&'");
            }
            else
            {
                args.push_back(tokens[i]);
            }
        }
    }
    void io_redirect()
    {
        if (input_file != "")
        {
            fd_in = open(input_file.c_str(), O_RDONLY);
            if (fd_in < 0)
            {
                perror("open");
                exit(1);
            }
            int ret = dup2(fd_in, STDIN_FILENO);
            if (ret < 0)
            {
                perror("dup2");
                exit(1);
            }
        }

        if (output_file != "")
        {
            fd_out = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0)
            {
                perror("open");
                exit(1);
            }
            int ret = dup2(fd_out, STDOUT_FILENO);
            if (ret < 0)
            {
                perror("dup2");
                exit(1);
            }
        }
    }
    friend ostream &operator<<(ostream &os, const Command &cmd)
    {
        for (auto it : cmd.args)
        {
            os << it << " ";
        }
        cout << endl;
        cout << "fd_in: " << cmd.fd_in << " fd_out: " << cmd.fd_out << endl;
        cout << "file in: " << cmd.input_file << " file out: " << cmd.output_file << endl;
        return os;
    }
};

void trim(string &s)
{
    while (s.length() && s.back() == ' ')
    {
        s.pop_back();
    }
    int i = 0;
    while (i < (int)s.length() && s[i] == ' ')
    {
        i++;
    }
    s = s.substr(i);
}

// Splits an input string on the basis of a delimiter
vector<string> split(string &str, char delim)
{
    vector<string> tokens;
    stringstream ss(str);
    string tmp;
    while (getline(ss, tmp, delim))
    {
        tokens.push_back(tmp);
    }
    return tokens;
}

void reapProcesses(int signum);
void toggleSIGCHLDBlock(int how);
void blockSIGCHLD();
void unblockSIGCHLD();
void waitForForegroundProcess(pid_t pid);
void CZ_handler(int signum);
void multiWatch_SIGINT(int signum);
// Reference: https://web.stanford.edu/class/archive/cs/cs110/cs110.1206/lectures/07-races-and-deadlock-slides.pdf
// Signal handler for SIGCHILD to prevent zombie processes

// These functions help in avoiding race conditions when SIGCHILD can be sent
void toggleSIGCHLDBlock(int how)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(how, &mask, NULL);
}

void blockSIGCHLD()
{
    toggleSIGCHLDBlock(SIG_BLOCK);
}

void unblockSIGCHLD()
{
    toggleSIGCHLDBlock(SIG_UNBLOCK);
}

// Ensures no race conditions for foreground processes
void waitForForegroundProcess(pid_t pid)
{
    fgpid = pid;
    sigset_t empty;

    sigemptyset(&empty);
    while (fgpid == pid)
    {
        sigsuspend(&empty);
    }
    unblockSIGCHLD();
}

// Signal handler for SIGINT and SIGTSTP
void CZ_handler(int signum)
{
    if (signum == SIGINT)
    {
        ctrlC = 1;
    }
    else if (signum == SIGTSTP)
    {
        ctrlZ = 1;
    }
}

int inotify_fd;          // The inotify instance to monitor the file descriptors
map<pid_t, int> pgid_wd; // Map process group id to watch descriptor
map<int, int> wd_ind;    // Map watch descriptor to index in the vector

// Signal handler for multiWatch in case of SIGINT
void multiWatch_SIGINT(int signum)
{
    for (auto it = pgid_wd.begin(); it != pgid_wd.end(); it++)
    {
        kill(-it->first, SIGINT);
    }
    close(inotify_fd);
}
// Removes whitespaces from beginning and end of a string
vector<Pipeline *> all_pipelines;
map<pid_t, int> ind;
class Pipeline
{
public:
    string cmd;             // The pipeline command as a string
    vector<Command *> cmds; // The component commands
    bool is_bg;             // Whether the pipeline is a background process
    pid_t pgid;             // The process group ID of processes in the pipeline
    int num_active;         // The number of active processes in the pipeline
    int status;             // The status of the pipeline - RUNNING, STOPPED, or DONE

    Pipeline(string &cmd) : cmd(cmd), is_bg(0), pgid(-1){};
    Pipeline(vector<Command *> &cmds) : cmds(cmds), is_bg(0), pgid(-1), num_active(cmds.size()), status(RUNNING){};
    void parse()
    {
        trim(this->cmd);
        if (this->cmd.back() == '&')
        {
            this->is_bg = true;
            this->cmd.pop_back();
        }
        vector<string> piped_cmds = split(this->cmd, '|'); // First, split the command on the basis of '|'

        try
        {
            vector<Command *> cmds;
            for (int i = 0; i < (int)piped_cmds.size(); i++)
            {
                trim(piped_cmds[i]);
                if (piped_cmds[i] == "")
                {
                    throw ShellException("Empty command in pipe");
                }
                Command *c = new Command(piped_cmds[i]);
                c->parse();
                if (c->args.size() == 0)
                {
                    throw ShellException("Empty command");
                }
                cmds.push_back(c);
            }
            this->cmds = cmds;
            this->num_active = cmds.size();
            this->status = RUNNING; // For simplicity, we directly set the status of the pipleing to RUNNING
        }
        catch (ShellException &e)
        {
            throw;
        }
    }
    // Converts a vector of strings to a vector of char*
    vector<char *> cstrArray(vector<string> &args)
    {
        vector<char *> args_(args.size() + 1);
        for (int i = 0; i < (int)args.size(); i++)
        {
            args_[i] = (char *)malloc((args[i].length() + 1) * sizeof(char));
            strcpy(args_[i], args[i].c_str());
        }
        args_[args.size()] = (char *)malloc(sizeof(char));
        args_[args.size()] = nullptr;
        return args_;
    }
    
    void executePipeline(bool isMultiwatch = false)
    {
        pid_t fg_pgid = 0;
        int new_pipe[2], old_pipe[2];

        blockSIGCHLD(); // Block SIGCHLD signal to avoid race conditions

        int cmds_size = this->cmds.size();
        for (int i = 0; i < cmds_size; i++)
        {
            if (i + 1 < cmds_size)
            {
                int ret = pipe(new_pipe); // Create the pipe
                if (ret < 0)
                {
                    perror("pipe");
                    throw ShellException("Unable to create pipe");
                }
            }
            pid_t cpid = fork();
            if (cpid < 0)
            {
                perror("fork");
                throw ShellException("Unable to fork");
            }
            if (cpid == 0)
            { // Child process
                unblockSIGCHLD();
                // Revert signal handlers to default behaviour
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);

                if (i == 0)
                {
                    fg_pgid = getpid(); // Get the pid of the first process to be set as the group id of all others in the pipeline
                }
                if (isMultiwatch && i + 1 == cmds_size)
                { // Create the tmp file in case of multiWatch
                    this->cmds[i]->output_file = ".tmp." + to_string(fg_pgid) + ".txt";
                }
                this->cmds[i]->io_redirect();

                if (i == 0)
                {
                    setpgrp(); // Set the process group id of the first process to be the same as its pid
                }
                else
                {
                    setpgid(0, fg_pgid);                     // Set the process group id of all child processes to be the same as the first process
                    dup2(old_pipe[0], this->cmds[i]->fd_in); // Input piping
                    close(old_pipe[0]);
                    close(old_pipe[1]);
                }
                if (i + 1 < cmds_size)
                {
                    dup2(new_pipe[1], this->cmds[i]->fd_out); // Output piping
                    close(new_pipe[0]);
                    close(new_pipe[1]);
                }

                if (this->cmds[i]->args[0] == "history")
                { // If the command is history, print the history
                    printHistory();
                    exit(0);
                }
                else
                {
                    vector<char *> c_args = cstrArray(this->cmds[i]->args);
                    execvp(c_args[0], c_args.data());
                    perror("execvp");
                    exit(1);
                }
            }
            else
            {                              // Parent process (shell)
                this->cmds[i]->pid = cpid; // Set the pid of the command
                if (i == 0)
                {
                    fg_pgid = cpid;
                    this->pgid = cpid;
                    setpgid(cpid, fg_pgid);        // Avoiding race conditions
                    all_pipelines.push_back(this); // Store this pipeline in the vector of all pipelines

                    // Reference: https://web.archive.org/web/20170701052127/https://www.usna.edu/Users/cs/aviv/classes/ic221/s16/lab/10/lab.html
                    // Give control of stdin to the running processes
                    tcsetpgrp(STDIN_FILENO, fg_pgid);
                }
                else
                {
                    setpgid(cpid, fg_pgid);
                }
                if (i > 0)
                {
                    close(old_pipe[0]);
                    close(old_pipe[1]);
                }
                old_pipe[0] = new_pipe[0];
                old_pipe[1] = new_pipe[1];

                ind[cpid] = (int)all_pipelines.size() - 1;
            }
        }

        if (this->is_bg || isMultiwatch)
        { // For background processes, we don't wait for them
            unblockSIGCHLD();
        }
        else
        {
            waitForForegroundProcess(fg_pgid);
            if (all_pipelines.back()->status == STOPPED)
            { // If Ctrl-Z was sent, now send SIGCONT to continue the process immediately in the background
                kill(-fg_pgid, SIGCONT);
            }
        }
        tcsetpgrp(STDIN_FILENO, getpid()); // Give control of stdin back to the shell
    }
    friend ostream &operator<<(ostream &os, const Pipeline &p)
    {
        cout << "p.pgid: " << p.pgid << endl;
        cout << "p.is_bg: " << p.is_bg << endl;
        for (auto it : p.cmds)
        {
            os << *it << endl;
        }
        return os;
    }
};

void reapProcesses(int signum)
{
    while (true)
    {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (pid <= 0)
        {
            break;
        }

        int id = ind[pid];
        Pipeline *pipeline = all_pipelines[id];
        if (WIFSIGNALED(status) || WIFEXITED(status))
        { // Terminated due to interrupt or normal exit
            pipeline->num_active--;
            if (pipeline->num_active == 0)
            {
                pipeline->status = DONE;
                if (pgid_wd.find(pipeline->pgid) != pgid_wd.end())
                { // Remove from watch list
                    inotify_rm_watch(inotify_fd, pgid_wd[pipeline->pgid]);
                }
            }
        }
        else if (WIFSTOPPED(status))
        { // Process was stopped (SIGTSTP)
            pipeline->num_active--;
            if (pipeline->num_active == 0)
            {
                pipeline->status = STOPPED;
            }
        }
        else if (WIFCONTINUED(status))
        { // Process was continued (SIGCONT)
            pipeline->num_active++;
            if (pipeline->num_active == (int)pipeline->cmds.size())
            {
                pipeline->status = RUNNING;
            }
        }

        if (pipeline->pgid == fgpid && !WIFCONTINUED(status))
        {
            if (pipeline->num_active == 0)
            {
                fgpid = 0; // To remove process from foreground
            }
        }
    }
}

#define HIST_SIZE 10000
#define HIST_DISPLAY_SIZE 1000

deque<string> history;

void loadHistory();
vector<string> searchInHistory(string s);

void addToHistory(string s);
void updateHistory();

const string HIST_FILE = ".shell_history";
map<pid_t, int> ind; // Mapping process id to index in the vector

// Load the history contents from the file when the shell starts
void loadHistory()
{
    history.clear();
    ifstream file(HIST_FILE);
    if (!file.is_open())
    {
        return;
    }
    else
    {
        string line = "";
        while (getline(file, line))
        {
            history.push_back(line);
        }
    }
    file.close();
}

// Returns the commands matched in the history
vector<string> searchInHistory(string s)
{
    vector<string> commands;
    for (int ind = history.size() - 1; ind >= 0; ind--)
    {
        // Substring matching is performed using the KMP algorithm
        string hist_cmd = history[ind];
        char ch = '\0' + 229;
        string t = s + ch + hist_cmd;
        int n = t.size();
        vector<int> lps(n + 1);
        int i = 0, j = -1;
        lps[0] = -1;
        while (i < n)
        {
            while (j != -1 && t[j] != t[i])
            {
                j = lps[j];
            }
            i++;
            j++;
            lps[i] = j;
            if (lps[i] == (int)s.size())
            {
                if (s.size() == hist_cmd.size())
                { // Exact match
                    return vector<string>(1, hist_cmd);
                }
                else
                { // Substring match
                    commands.push_back(hist_cmd);
                }
            }
        }
    }
    return (s.size() > 2 ? commands : vector<string>());
}

// Display the shell history when the 'history' command has to be executed
void printHistory()
{
    int i = max(0, (int)history.size() - HIST_DISPLAY_SIZE);
    for (int cnt = 0; cnt < min((int)history.size(), HIST_DISPLAY_SIZE); i++, cnt++)
    {
        cout << i + 1 << " " << history[i] << endl;
    }
}

// Add a command to the history
void addToHistory(string s)
{
    if (history.size() == HIST_SIZE)
    {
        history.pop_front();
    }
    history.push_back(s);
}

// Save the history to the file when the shell exits
void updateHistory()
{
    ofstream file(HIST_FILE);
    if (!file.is_open())
    {
        return;
    }
    else
    {
        for (auto it : history)
        {
            string temp = it + "\n";
            file << temp;
        }
    }
    file.close();
}

// Returns a vector of strings containing the names of all files in the current directory
vector<string> getFilesInCurrDir()
{
    vector<string> files;
    DIR *dir = opendir(".");
    if (dir)
    {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {                                          // Navigate the directory using readdir
            DIR *sub_dir = opendir(entry->d_name); // If the entry cannot be opened, it means it is a file
            if (sub_dir)
            {
                closedir(sub_dir);
            }
            else
            {
                string file(entry->d_name);
                files.push_back(file);
            }
        }
        closedir(dir);
    }
    else
    {
        cout << "Directory could not be opened";
    }
    return files;
}

// Returns the autocomplete suggestions for a query string
vector<string> autocomplete(string s)
{
    vector<string> filenames = getFilesInCurrDir();
    vector<string> matched;
    int s_len = s.length();
    for (int i = 0; i < (int)filenames.size(); i++)
    {
        if ((int)filenames[i].length() < s_len)
        {
            continue;
        }
        else if (s == filenames[i].substr(0, s_len))
        { // Check if s is a prefix of filenames[i]
            matched.push_back(filenames[i]);
        }
    }

    if ((int)matched.size() <= 1)
    { // Single match
        return matched;
    }
    else
    {
        // Try to find a common prefix in all the suggestions matched
        sort(matched.begin(), matched.end());
        int i = 0;
        string suggest = "";
        while (i < (int)min(matched.front().size(), matched.back().size()) && matched.front()[i] == matched.back()[i])
        {
            suggest += matched[0][i++];
        }
        if (suggest.length() > s.length())
        {
            return vector<string>(1, suggest);
        }
        else
        {
            return matched;
        }
    }
}

// Displays the prompt for the shell
void displayPrompt()
{
    char buf[DIR_LENGTH];
    getcwd(buf, DIR_LENGTH);
    string dir(buf);
    dir = dir.substr(dir.find_last_of("/") + 1);
    cout << COLOR_GREEN << "vash: " << COLOR_BLUE << dir << COLOR_RESET << "$ ";
}

// Performs necessay actions for each character input
int handleChar(char c, string &buf)
{
    if (c == CTRL_CZ)
    { // Ctrl + C, Ctrl + Z
        if (ctrlC)
        {
            printf("\n");
            buf = "";
            ctrlC = 0;
            return 2;
        }
        else if (ctrlZ)
        {
            ctrlZ = 0;
            return 0;
        }
    }
    else if (c == CTRL_D)
    { // Ctrl + D
        ctrlD = 1;
        buf = "";
        return 4;
    }
    else if (c == BACKSPACE)
    { // Backspace
        if (buf.length() > 0)
        {
            printf("\b \b");
            buf.pop_back();
        }
        return 0;
    }
    else if (c == ENTER)
    { // Enter
        printf("\n");
        return 1;
    }
    else if (c > 31 && c < 127)
    { // Printable characters
        printf("%c", c);
        buf += c;
        return 0;
    }
    return 0;
}

// Reads a command from the user (with autocomplete and history search)
string readCommand()
{
    struct termios old_tio, new_tio;
    signed char c;

    // Get the terminal settings for stdin
    tcgetattr(STDIN_FILENO, &old_tio);

    // We want to keep the old setting to restore them at the end
    new_tio = old_tio;

    // Disable canonical mode (bufered i/o) and local echo
    new_tio.c_lflag &= (~ICANON & ~ECHO);
    new_tio.c_cc[VMIN] = 1;
    new_tio.c_cc[VTIME] = 0;

    // Set the new settings immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

    string buf;
    while (1)
    {
        int ret_in, ret_out;
        c = getchar();
        if (c == CTRL_R)
        { // Ctrl + R (history search)
            printf("\n");
            printf("Enter search term: ");
            string s;
            while (1)
            {
                char ch = getchar();
                ret_in = handleChar(ch, s);
                if (ret_in != 0)
                {
                    break;
                }
            }
            if (ret_in == 4)
            { // Ctrl + D
                ctrlD = 0;
                buf = "";
                break;
            }
            else if (ret_in == 2)
            { // Ctrl + C
                buf = "";
                break;
            }

            vector<string> cmds = searchInHistory(s);
            if (cmds.size() == 0)
            {
                printf("No match for search term in history\n");
                displayPrompt();
            }
            else if (cmds.size() == 1 && cmds[0] == s)
            { // exact match
                printf("Exact match found in history: %s\n", cmds[0].c_str());
                displayPrompt();
                buf = cmds[0];
                printf("%s", buf.c_str());
            }
            else
            {
                printf("Similar commands matched (most recent first):\n");
                for (int i = 0; i < (int)cmds.size(); i++)
                {
                    printf("%s\n", cmds[i].c_str());
                }
                displayPrompt();
            }
        }
        else if (c == TAB)
        { // Tab (auto completion)
            string s;
            set<char> delims = {' ', '\t', '/', '>', '<', '|'};
            for (int j = (int)buf.length() - 1; j >= 0; j--)
            { // Extract the last word
                if (delims.find(buf[j]) != delims.end())
                {
                    break;
                }
                s += buf[j];
            }
            reverse(s.begin(), s.end());
            vector<string> complete_options = autocomplete(s);
            if (complete_options.size() == 0)
            {
                continue;
            }
            else if (complete_options.size() == 1)
            {
                string str = complete_options[0].substr(s.length());
                buf += str;
                printf("%s", str.c_str());
            }
            else
            {
                printf("\n");
                for (int j = 0; j < (int)complete_options.size(); j++)
                {
                    printf("%d. %s ", j + 1, complete_options[j].c_str());
                }
                printf("\n");
                printf("Enter choice: "); // Take choice as input in case of multiple options
                string n;
                while (1)
                {
                    char ch = getchar();
                    ret_in = handleChar(ch, n);
                    if (ret_in != 0)
                    {
                        break;
                    }
                }
                if (ret_in == 4)
                { // Ctrl + D
                    ctrlD = 0;
                    buf = "";
                    break;
                }
                else if (ret_in == 2)
                { // Ctrl + C
                    buf = "";
                    break;
                }
                try
                {
                    int num = stoi(n);
                    if (num > (int)complete_options.size() || num <= 0)
                    {
                        printf("Invalid Choice\n");
                    }
                    else
                    {
                        buf += complete_options[num - 1].substr(s.length());
                    }
                }
                catch (...)
                {
                    printf("Invalid Choice\n");
                }
                displayPrompt();
                printf("%s", buf.c_str());
            }
        }
        else
        {
            ret_out = handleChar(c, buf);
            if (ret_out != 0)
            {
                break;
            }
        }
    }

    // Restore the former settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

    return buf;
}

vector<Pipeline *> parseMultiWatch(string cmd, string &output_file);
void executeMultiWatch(vector<Pipeline *> &pList, string output_file = "");

// Creates pipelines for all commands inside quotes for multiWatch
vector<Pipeline *> parseMultiWatch(string cmd, string &output_file)
{
    trim(cmd);
    if (cmd.length() < 10)
    { // Not multiWatch
        return vector<Pipeline *>();
    }
    vector<Pipeline *> pipelines;
    if (cmd.substr(0, 10) == "multiWatch")
    {
        cmd = cmd.substr(10);
        trim(cmd);
        if (cmd.length())
        {
            if (cmd.back() != ']')
            {
                int i = cmd.length() - 1;
                while (i >= 0 && cmd[i] != ']')
                {
                    i--;
                }
                if (i == -1)
                {
                    throw ShellException("Could not find ']");
                }
                else
                {
                    // Extract output filename if present
                    output_file = cmd.substr(i + 1);
                    cmd = cmd.substr(0, i + 1);
                    trim(output_file);
                    if (output_file.length())
                    {
                        if (output_file[0] == '>')
                        {
                            output_file = output_file.substr(1);
                            trim(output_file);
                            if (output_file.length() == 0)
                            {
                                throw ShellException("No output file after >");
                            }
                        }
                        else
                        {
                            throw ShellException("Unable to parse after multiWatch");
                        }
                    }
                }
            }
            trim(cmd);
            cmd.pop_back();
            if (cmd[0] == '[')
            {
                cmd = cmd.substr(1);
                vector<string> commands = split(cmd, ',');
                for (auto &command : commands)
                {
                    trim(command);
                    if (command[0] == '\"' && command.back() == '\"')
                    {
                        command = command.substr(1, command.length() - 2);
                    }
                    else
                    {
                        throw ShellException("No quotes around command");
                    }
                }
                if (commands.empty())
                {
                    throw ShellException("No commands in multiWatch");
                }
                for (auto &command : commands)
                {
                    if (command != "")
                    {
                        Pipeline *p = new Pipeline(command); // Create a pipeline for each string inside quotes
                        p->parse();
                        pipelines.push_back(p);
                    }
                    else
                    {
                        cout << "Empty command in multiWatch" << endl;
                    }
                }
            }
            else
            {
                throw ShellException("Could not find '[");
            }
        }
        else
        {
            throw ShellException("No commands after multiWatch");
        }
    }
    else
    {
        return vector<Pipeline *>(); // Signifies multiWatch is not present
    }
    return pipelines;
}

// Executes the multiWatch command by creating an inotify instance and monitoring the tmp files for output
void executeMultiWatch(vector<Pipeline *> &pList, string output_file)
{
    inotify_fd = inotify_init(); // Initialize inotify instance
    if (inotify_fd < 0)
    {
        perror("inotify_init");
        throw ShellException("Unable to create inotify instance");
    }

    vector<int> fds; // To store file descriptos from the tmp files to read
    for (int i = 0; i < (int)pList.size(); i++)
    {
        pList[i]->executePipeline(true);
        string tmpFile = ".tmp." + to_string(pList[i]->pgid) + ".txt";
        int fd = open(tmpFile.c_str(), O_RDONLY);
        if (fd < 0)
        {
            // Handle race condition of file not being already created in the child process
            int temp_fd = open(tmpFile.c_str(), O_CREAT, 0644);
            if (temp_fd < 0)
            {
                perror("open");
                throw ShellException("Unable to open tmp file");
            }
            close(temp_fd);
            fd = open(tmpFile.c_str(), O_RDONLY);
        }
        fds.push_back(fd);
        int wd = inotify_add_watch(inotify_fd, tmpFile.c_str(), IN_MODIFY); // Add watch descriptor to inotify instance
        if (wd < 0)
        {
            perror("inotify_add_watch");
            throw ShellException("Unable to add watch");
        }
        pgid_wd[pList[i]->pgid] = wd; // Map process group id to watch descriptor
        wd_ind[wd] = i;               // Map watch descriptor to index in the vector
    }

    int num_running = pList.size();

    FILE *out_fp;
    if (output_file == "")
    {
        out_fp = stdout;
    }
    else
    {
        // Open output file for writing (if anything other than stdout)
        out_fp = fopen(output_file.c_str(), "w");
        if (out_fp == NULL)
        {
            perror("fopen");
            throw ShellException("Unable to open output file");
        }
    }

    int event_size = sizeof(struct inotify_event);
    int event_buf_len = 1024 * (event_size + 16);
    char event_buf[event_buf_len] __attribute__((aligned(__alignof__(struct inotify_event))));

    char buf[1024];

    // Continue watching the file descriptors until all processes are done
    while (num_running)
    {
        int num_read = read(inotify_fd, event_buf, sizeof(event_buf)); // Get the next event
        if (num_read < 0 && errno != EINTR && errno != EBADF)
        {
            perror("read");
            throw ShellException("Unable to read from inotify instance");
        }
        else if (errno == EBADF)
        { // When inotify instance has been closed
            break;
        }

        int i = 0;
        while (i < num_read)
        {
            struct inotify_event *event = (struct inotify_event *)&event_buf[i];
            // IN_MODIFY implies something has been written to the file
            // IN_IGNORED implies the watch desriptor was removed from the watch list
            if ((event->mask & IN_MODIFY) || (event->mask & IN_IGNORED))
            {
                int fd = fds[wd_ind[event->wd]];
                int len, seen_data = 0;
                while ((len = read(fd, buf, sizeof(buf))) > 0 || errno == EINTR)
                {
                    if (!seen_data)
                    {
                        int sz = pList[wd_ind[event->wd]]->cmd.length() + to_string(time(NULL)).length() + 6;
                        string line(sz, '-');
                        fprintf(out_fp, "%s\n\"%s\", %ld :\n%s\n", line.c_str(), pList[wd_ind[event->wd]]->cmd.c_str(), time(NULL), line.c_str());
                        fflush(out_fp);
                        seen_data = 1;
                    }
                    buf[len] = '\0';
                    fprintf(out_fp, "%s", buf); // Write to output file
                    fflush(out_fp);
                }
            }
            if (event->mask & IN_IGNORED)
            {
                // The watch descriptor is removed in the signal handler when the process is finished and IN_IGNORED is set
                num_running--;
                close(fds[wd_ind[event->wd]]);
            }
            i += event_size + event->len;
        }
    }

    // Delete the tmp files
    for (auto it = pgid_wd.begin(); it != pgid_wd.end(); it++)
    {
        string tmpFile = ".tmp." + to_string(it->first) + ".txt";
        if (remove(tmpFile.c_str()) < 0)
        {
            perror("remove");
            throw ShellException("Unable to remove tmp file");
        }
    }
    pgid_wd.clear();
    close(inotify_fd);
    if (out_fp != stdout)
    {
        fclose(out_fp);
    }
}

// To handle the cd builitin
void shellCd(string arg)
{
    trim(arg);
    if (arg == "")
    {
        throw ShellException("No directory specified");
    }
    if (chdir(arg.c_str()) < 0)
    {
        perror("chdir");
    }
}

// To handle the exit builitin - exits the shell
void shellExit(string arg)
{
    updateHistory();
    exit(0);
}

// To handle the jobs bulitin - lists all the jobs
void shellJobs(string arg)
{
    for (int i = 0; i < (int)all_pipelines.size(); i++)
    {
        cout << "pgid: " << all_pipelines[i]->pgid << ": ";
        int status = all_pipelines[i]->status;
        if (status == RUNNING)
        {
            cout << "Running";
        }
        else if (status == STOPPED)
        {
            cout << "Stopped";
        }
        else if (status == DONE)
        {
            cout << "Done";
        }
        cout << endl;

        vector<Command *> cmds = all_pipelines[i]->cmds;
        for (int j = 0; j < (int)cmds.size(); j++)
        {
            cout << "--- pid: " << cmds[j]->pid << " " << cmds[j]->cmd << endl;
        }
    }
}

vector<string> builtins = {"cd", "exit", "jobs"};
void (*builtin_funcs[])(string args) = {&shellCd, &shellExit, &shellJobs};

// Handles the builtin commands - cd, exit, jobs
void handleBuiltin(Pipeline &p)
{
    try
    {
        string cmd = p.cmds[0]->args[0];
        for (int i = 0; i < (int)builtins.size(); i++)
        {
            if (cmd == builtins[i])
            {
                string arg = (p.cmds[0]->args.size() > 1) ? p.cmds[0]->args[1] : "";
                (*builtin_funcs[i])(arg);
            }
        }
    }
    catch (ShellException &e)
    {
        throw;
    }
}

int main()
{
    // Loading history when shell starts
    loadHistory();

    // Specify signal handlers
    struct sigaction action;
    action.sa_handler = CZ_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);

    // Reference: https://web.archive.org/web/20170701052127/https://www.usna.edu/Users/cs/aviv/classes/ic221/s16/lab/10/lab.html
    signal(SIGTTOU, SIG_IGN);

    // Reference: https://web.stanford.edu/class/archive/cs/cs110/cs110.1206/lectures/07-races-and-deadlock-slides.pdf
    signal(SIGCHLD, reapProcesses);

    while (!ctrlD)
    {
        displayPrompt();
        string cmd = readCommand();
        trim(cmd);
        if (cmd == "")
        {
            continue;
        }
        addToHistory(cmd);

        try
        {
            string output_file = "";
            vector<Pipeline *> pList = parseMultiWatch(cmd, output_file); // Try parsing for multiWatch
            if (pList.size() > 0)
            { // multiWatch detected
                // Change signal handlers in case of multiWatch
                struct sigaction multiWatch_action;
                multiWatch_action.sa_handler = multiWatch_SIGINT;
                sigemptyset(&multiWatch_action.sa_mask);
                multiWatch_action.sa_flags = 0;
                sigaction(SIGINT, &multiWatch_action, NULL);
                signal(SIGTSTP, SIG_IGN);

                executeMultiWatch(pList, output_file);

                // Revert back to signal handlers for the shell
                sigaction(SIGINT, &action, NULL);
                sigaction(SIGTSTP, &action, NULL);
            }
            else
            {
                if (cmd.size() >= 10 && cmd.substr(0, 10) == "multiWatch")
                {
                    throw ShellException("Error while parsing command");
                }
                Pipeline *p = new Pipeline(cmd);
                p->parse();
                string arg = p->cmds[0]->args[0];
                if (arg == "cd" || arg == "exit" || arg == "jobs")
                {
                    handleBuiltin(*p);
                    continue;
                }
                p->executePipeline(); // Execute the pipeline
            }
        }
        catch (ShellException &e)
        {
            cout << e.what() << endl;
        }
    }

    // Save history when shell exits
    updateHistory();
}
