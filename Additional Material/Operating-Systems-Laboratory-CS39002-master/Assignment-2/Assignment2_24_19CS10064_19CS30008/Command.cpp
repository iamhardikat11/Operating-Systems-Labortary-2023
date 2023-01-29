#include "Command.h"

#include <fcntl.h>
#include <unistd.h>

#include "ShellException.h"

using namespace std;

Command::Command(const string& cmd) : cmd(cmd), fd_in(STDIN_FILENO), fd_out(STDOUT_FILENO), input_file(""), output_file("") {}

Command::~Command() {
    if (fd_in != STDIN_FILENO) {
        close(fd_in);
    }
    if (fd_out != STDOUT_FILENO) {
        close(fd_out);
    }
}

// Parses the command string into a vector of arguments
void Command::parse() {
    vector<string> tokens;
    string temp = "";
    for (size_t i = 0; i < cmd.length(); i++) {
        if (cmd[i] == '\\') {  // Escape character
            i++;
            if (i != cmd.length()) {
                temp += cmd[i];
            } else {
                throw ShellException("Unable to parse '\\'");
            }
            continue;
        }
        if (cmd[i] == '>' || cmd[i] == '<' || cmd[i] == '&') {
            if (temp.size() > 0) {
                tokens.push_back(temp);
                temp = "";
            }
            tokens.push_back(string(1, cmd[i]));
        } else if (cmd[i] == '"') {
            i++;
            while (i < cmd.length() && (cmd[i] != '"' || (cmd[i] == '"' && cmd[i - 1] == '\\'))) {
                temp += cmd[i++];
            }
            if (i == cmd.length()) {
                throw ShellException("Unable to parse '\"'");
            }
        } else if (cmd[i] == ' ') {
            if (temp.size() > 0) {
                tokens.push_back(temp);
                temp = "";
            }
        } else {
            temp += cmd[i];
        }
    }
    if (temp.size() > 0) {
        tokens.push_back(temp);
    }

    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i] == "<") {  // Input redirection
            i++;
            if (i == tokens.size() || tokens[i] == ">" || tokens[i] == "<" || tokens[i] == "&") {
                throw ShellException("Unable to parse after '<'");
            } else {
                input_file = tokens[i];
            }
        } else if (tokens[i] == ">") {  // Output redirection
            i++;
            if (i == tokens.size() || tokens[i] == ">" || tokens[i] == "<" || tokens[i] == "&") {
                throw ShellException("Unable to parse after '>'");
            } else {
                output_file = tokens[i];
            }
        } else if (tokens[i] == "&") {  // & is already parsed at the pipeline parsing stage
            throw ShellException("Unable to parse after '&'");
        } else {
            args.push_back(tokens[i]);
        }
    }
}

// Change the I/O file descriptors for redirection
void Command::io_redirect() {
    if (input_file != "") {
        fd_in = open(input_file.c_str(), O_RDONLY);
        if (fd_in < 0) {
            perror("open");
            exit(1);
        }
        int ret = dup2(fd_in, STDIN_FILENO);
        if (ret < 0) {
            perror("dup2");
            exit(1);
        }
    }

    if (output_file != "") {
        fd_out = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out < 0) {
            perror("open");
            exit(1);
        }
        int ret = dup2(fd_out, STDOUT_FILENO);
        if (ret < 0) {
            perror("dup2");
            exit(1);
        }
    }
}

ostream& operator<<(ostream& os, const Command& cmd) {
    for (auto it : cmd.args) {
        os << it << " ";
    }
    cout << endl;
    cout << "fd_in: " << cmd.fd_in << " fd_out: " << cmd.fd_out << endl;
    cout << "file in: " << cmd.input_file << " file out: " << cmd.output_file << endl;
    return os;
}
