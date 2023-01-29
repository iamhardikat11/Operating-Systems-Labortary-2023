#include "shell.h"

#include <signal.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <deque>
#include <map>

#include "Command.h"
#include "Pipeline.h"
#include "ShellException.h"
#include "history.h"
#include "multiWatch.h"
#include "read_command.h"
#include "signal_handlers.h"
#include "utility.h"

using namespace std;

bool ctrlC = 0, ctrlZ = 0, ctrlD = 0;  // Indicates whether the user has pressed Ctrl-C, Ctrl-Z, or Ctrl-D
pid_t fgpid = 0;                       // Foreground process group id

vector<Pipeline*> all_pipelines;  // To store all the pipelines in the shell
map<pid_t, int> ind;              // Mapping process id to index in the vector

// To handle the cd builitin
void shellCd(string arg) {
    trim(arg);
    if (arg == "") {
        throw ShellException("No directory specified");
    }
    if (chdir(arg.c_str()) < 0) {
        perror("chdir");
    }
}

// To handle the exit builitin - exits the shell
void shellExit(string arg) {
    updateHistory();
    exit(0);
}

// To handle the jobs bulitin - lists all the jobs
void shellJobs(string arg) {
    for (int i = 0; i < (int)all_pipelines.size(); i++) {
        cout << "pgid: " << all_pipelines[i]->pgid << ": ";
        int status = all_pipelines[i]->status;
        if (status == RUNNING) {
            cout << "Running";
        } else if (status == STOPPED) {
            cout << "Stopped";
        } else if (status == DONE) {
            cout << "Done";
        }
        cout << endl;

        vector<Command*> cmds = all_pipelines[i]->cmds;
        for (int j = 0; j < (int)cmds.size(); j++) {
            cout << "--- pid: " << cmds[j]->pid << " " << cmds[j]->cmd << endl;
        }
    }
}

vector<string> builtins = {"cd", "exit", "jobs"};
void (*builtin_funcs[])(string args) = {&shellCd, &shellExit, &shellJobs};

// Handles the builtin commands - cd, exit, jobs
void handleBuiltin(Pipeline& p) {
    try {
        string cmd = p.cmds[0]->args[0];
        for (int i = 0; i < (int)builtins.size(); i++) {
            if (cmd == builtins[i]) {
                string arg = (p.cmds[0]->args.size() > 1) ? p.cmds[0]->args[1] : "";
                (*builtin_funcs[i])(arg);
            }
        }
    } catch (ShellException& e) {
        throw;
    }
}

int main() {
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

    while (!ctrlD) {
        displayPrompt();
        string cmd = readCommand();
        trim(cmd);
        if (cmd == "") {
            continue;
        }
        addToHistory(cmd);

        try {
            string output_file = "";
            vector<Pipeline*> pList = parseMultiWatch(cmd, output_file);  // Try parsing for multiWatch
            if (pList.size() > 0) {                                       // multiWatch detected
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
            } else {
                if (cmd.size() >= 10 && cmd.substr(0, 10) == "multiWatch") {
                    throw ShellException("Error while parsing command");
                }
                Pipeline* p = new Pipeline(cmd);
                p->parse();
                string arg = p->cmds[0]->args[0];
                if (arg == "cd" || arg == "exit" || arg == "jobs") {
                    handleBuiltin(*p);
                    continue;
                }
                p->executePipeline();   // Execute the pipeline
            }
        } catch (ShellException& e) {
            cout << e.what() << endl;
        }
    }

    // Save history when shell exits
    updateHistory();
}
