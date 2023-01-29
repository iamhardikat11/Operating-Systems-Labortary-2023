#ifndef __SIGNAL_HANDLERS_H
#define __SIGNAL_HANDLERS_H

#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <vector>

#include "Pipeline.h"

using namespace std;

extern bool ctrlC, ctrlZ, ctrlD;
extern pid_t fgpid;

extern vector<Pipeline*> all_pipelines;
extern map<pid_t, int> ind;

extern int inotify_fd;
extern map<pid_t, int> pgid_wd;

void reapProcesses(int signum);
void toggleSIGCHLDBlock(int how);
void blockSIGCHLD();
void unblockSIGCHLD();
void waitForForegroundProcess(pid_t pid);
void CZ_handler(int signum);
void multiWatch_SIGINT(int signum);

#endif