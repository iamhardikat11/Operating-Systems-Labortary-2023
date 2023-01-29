#include "signal_handlers.h"

#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <map>

using namespace std;

// Reference: https://web.stanford.edu/class/archive/cs/cs110/cs110.1206/lectures/07-races-and-deadlock-slides.pdf
// Signal handler for SIGCHILD to prevent zombie processes
void reapProcesses(int signum) {
    while (true) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (pid <= 0) {
            break;
        }

        int id = ind[pid];
        Pipeline* pipeline = all_pipelines[id];
        if (WIFSIGNALED(status) || WIFEXITED(status)) {  // Terminated due to interrupt or normal exit
            pipeline->num_active--;
            if (pipeline->num_active == 0) {
                pipeline->status = DONE;
                if (pgid_wd.find(pipeline->pgid) != pgid_wd.end()) {  // Remove from watch list
                    inotify_rm_watch(inotify_fd, pgid_wd[pipeline->pgid]);
                }
            }
        } else if (WIFSTOPPED(status)) {    // Process was stopped (SIGTSTP)
            pipeline->num_active--;
            if (pipeline->num_active == 0) {
                pipeline->status = STOPPED;
            }
        } else if (WIFCONTINUED(status)) {  // Process was continued (SIGCONT)
            pipeline->num_active++;
            if (pipeline->num_active == (int)pipeline->cmds.size()) {
                pipeline->status = RUNNING;
            }
        }

        if (pipeline->pgid == fgpid && !WIFCONTINUED(status)) {
            if (pipeline->num_active == 0) {
                fgpid = 0;  // To remove process from foreground
            }
        }
    }
}

// These functions help in avoiding race conditions when SIGCHILD can be sent
void toggleSIGCHLDBlock(int how) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(how, &mask, NULL);
}

void blockSIGCHLD() {
    toggleSIGCHLDBlock(SIG_BLOCK);
}

void unblockSIGCHLD() {
    toggleSIGCHLDBlock(SIG_UNBLOCK);
}

// Ensures no race conditions for foreground processes
void waitForForegroundProcess(pid_t pid) {
    fgpid = pid;
    sigset_t empty;

    sigemptyset(&empty);
    while (fgpid == pid) {
        sigsuspend(&empty);
    }
    unblockSIGCHLD();
}

// Signal handler for SIGINT and SIGTSTP
void CZ_handler(int signum) {
    if (signum == SIGINT) {
        ctrlC = 1;
    } else if (signum == SIGTSTP) {
        ctrlZ = 1;
    }
}

// Signal handler for multiWatch in case of SIGINT
void multiWatch_SIGINT(int signum) {
    for (auto it = pgid_wd.begin(); it != pgid_wd.end(); it++) {
        kill(-it->first, SIGINT);
    }
    close(inotify_fd);
}