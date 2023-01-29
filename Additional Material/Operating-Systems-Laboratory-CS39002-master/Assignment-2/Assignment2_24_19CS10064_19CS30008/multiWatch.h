#ifndef __MULTIWATCH_H
#define __MULTIWATCH_H

#include <map>
#include <string>
#include <vector>

#include "Pipeline.h"

using namespace std;

extern int inotify_fd;
extern map<pid_t, int> pgid_wd;
extern map<int, int> wd_ind;

vector<Pipeline*> parseMultiWatch(string cmd, string& output_file);
void executeMultiWatch(vector<Pipeline*>& pList, string output_file = "");

#endif