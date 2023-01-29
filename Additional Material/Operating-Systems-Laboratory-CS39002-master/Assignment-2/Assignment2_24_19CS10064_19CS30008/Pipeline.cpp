#include "Pipeline.h"

#include <signal.h>
#include <sys/signal.h>
#include <unistd.h>

#include "ShellException.h"
#include "history.h"
#include "signal_handlers.h"
#include "utility.h"

using namespace std;

Pipeline::Pipeline(string& cmd) : cmd(cmd), is_bg(0), pgid(-1) {}

Pipeline::Pipeline(vector<Command*>& cmds) : cmds(cmds), is_bg(0), pgid(-1), num_active(cmds.size()), status(RUNNING) {}

// Parses the command string into a vector of Command objects
void Pipeline::parse() {
    trim(this->cmd);
    if (this->cmd.back() == '&') {
        this->is_bg = true;
        this->cmd.pop_back();
    }
    vector<string> piped_cmds = split(this->cmd, '|');  // First, split the command on the basis of '|'

    try {
        vector<Command*> cmds;
        for (int i = 0; i < (int)piped_cmds.size(); i++) {
            trim(piped_cmds[i]);
            if (piped_cmds[i] == "") {
                throw ShellException("Empty command in pipe");
            }
            Command* c = new Command(piped_cmds[i]);
            c->parse();
            if (c->args.size() == 0) {
                throw ShellException("Empty command");
            }
            cmds.push_back(c);
        }
        this->cmds = cmds;
        this->num_active = cmds.size();
        this->status = RUNNING;  // For simplicity, we directly set the status of the pipleing to RUNNING
    } catch (ShellException& e) {
        throw;
    }
}

// Excetutes all the commands in the pipeline
void Pipeline::executePipeline(bool isMultiwatch) {
    pid_t fg_pgid = 0;
    int new_pipe[2], old_pipe[2];

    blockSIGCHLD();  // Block SIGCHLD signal to avoid race conditions

    int cmds_size = this->cmds.size();
    for (int i = 0; i < cmds_size; i++) {
        if (i + 1 < cmds_size) {
            int ret = pipe(new_pipe);  // Create the pipe
            if (ret < 0) {
                perror("pipe");
                throw ShellException("Unable to create pipe");
            }
        }
        pid_t cpid = fork();
        if (cpid < 0) {
            perror("fork");
            throw ShellException("Unable to fork");
        }
        if (cpid == 0) {  // Child process
            unblockSIGCHLD();
            // Revert signal handlers to default behaviour
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if (i == 0) {
                fg_pgid = getpid();  // Get the pid of the first process to be set as the group id of all others in the pipeline
            }
            if (isMultiwatch && i + 1 == cmds_size) {  // Create the tmp file in case of multiWatch
                this->cmds[i]->output_file = ".tmp." + to_string(fg_pgid) + ".txt";
            }
            this->cmds[i]->io_redirect();

            if (i == 0) {
                setpgrp();  // Set the process group id of the first process to be the same as its pid
            } else {
                setpgid(0, fg_pgid);                      // Set the process group id of all child processes to be the same as the first process
                dup2(old_pipe[0], this->cmds[i]->fd_in);  // Input piping
                close(old_pipe[0]);
                close(old_pipe[1]);
            }
            if (i + 1 < cmds_size) {
                dup2(new_pipe[1], this->cmds[i]->fd_out);  // Output piping
                close(new_pipe[0]);
                close(new_pipe[1]);
            }

            if (this->cmds[i]->args[0] == "history") {  // If the command is history, print the history
                printHistory();
                exit(0);
            } else {
                vector<char*> c_args = cstrArray(this->cmds[i]->args);
                execvp(c_args[0], c_args.data());
                perror("execvp");
                exit(1);
            }
        } else {                        // Parent process (shell)
            this->cmds[i]->pid = cpid;  // Set the pid of the command
            if (i == 0) {
                fg_pgid = cpid;
                this->pgid = cpid;
                setpgid(cpid, fg_pgid);         // Avoiding race conditions
                all_pipelines.push_back(this);  // Store this pipeline in the vector of all pipelines

                // Reference: https://web.archive.org/web/20170701052127/https://www.usna.edu/Users/cs/aviv/classes/ic221/s16/lab/10/lab.html
                // Give control of stdin to the running processes
                tcsetpgrp(STDIN_FILENO, fg_pgid);
            } else {
                setpgid(cpid, fg_pgid);
            }
            if (i > 0) {
                close(old_pipe[0]);
                close(old_pipe[1]);
            }
            old_pipe[0] = new_pipe[0];
            old_pipe[1] = new_pipe[1];

            ind[cpid] = (int)all_pipelines.size() - 1;
        }
    }

    if (this->is_bg || isMultiwatch) {  // For background processes, we don't wait for them
        unblockSIGCHLD();
    } else {
        waitForForegroundProcess(fg_pgid);
        if (all_pipelines.back()->status == STOPPED) {  // If Ctrl-Z was sent, now send SIGCONT to continue the process immediately in the background
            kill(-fg_pgid, SIGCONT);
        }
    }
    tcsetpgrp(STDIN_FILENO, getpid());  // Give control of stdin back to the shell
}

ostream& operator<<(ostream& os, const Pipeline& p) {
    cout << "p.pgid: " << p.pgid << endl;
    cout << "p.is_bg: " << p.is_bg << endl;
    for (auto it : p.cmds) {
        os << *it << endl;
    }
    return os;
}
