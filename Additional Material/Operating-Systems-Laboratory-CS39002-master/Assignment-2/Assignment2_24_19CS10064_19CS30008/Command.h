#ifndef __COMMAND_H
#define __COMMAND_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Command {
   public:
    string cmd;                      // The command as a string
    vector<string> args;             // List of arguments in the command
    int fd_in, fd_out;               // Input and output file descriptors
    string input_file, output_file;  // Input and output filenames
    pid_t pid;                       // Process ID of the command when it executes

    Command(const string& cmd);
    ~Command();

    void parse();
    void io_redirect();
    friend ostream& operator<<(ostream& os, const Command& cmd);
};

#endif