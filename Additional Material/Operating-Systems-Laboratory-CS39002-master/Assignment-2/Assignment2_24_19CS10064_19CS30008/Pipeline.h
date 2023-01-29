#ifndef __PIPELINE_H
#define __PIPELINE_H

#include <string>
#include <vector>

#include "Command.h"

using namespace std;

#define RUNNING 0
#define STOPPED 1
#define DONE 2

class Pipeline {
   public:
    string cmd;             // The pipeline command as a string
    vector<Command*> cmds;  // The component commands
    bool is_bg;             // Whether the pipeline is a background process
    pid_t pgid;             // The process group ID of processes in the pipeline
    int num_active;         // The number of active processes in the pipeline
    int status;             // The status of the pipeline - RUNNING, STOPPED, or DONE

    Pipeline(string& cmd);
    Pipeline(vector<Command*>& cmds);
    void parse();
    void executePipeline(bool isMultiwatch = false);
    friend ostream& operator<<(ostream& os, const Pipeline& p);
};

#endif