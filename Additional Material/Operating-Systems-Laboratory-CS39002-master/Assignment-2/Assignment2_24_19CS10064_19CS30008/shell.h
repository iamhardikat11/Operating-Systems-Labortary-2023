#ifndef __SHELL_H
#define __SHELL_H

#include <string>

#include "Pipeline.h"

using namespace std;

void shellCd(string arg);
void shellExit(string arg = "");
void shellJobs(string arg = "");
void handleBuiltin(Pipeline& p);

#endif