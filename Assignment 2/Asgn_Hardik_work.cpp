#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <unistd.h>
#include <algorithm>
#include <queue>
#include <fstream>
using namespace std;

#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN "\033[1;36m"
#define COLOR_RESET "\033[0m"

#define CTRL_CZ -1
#define CTRL_D 4
#define CTRL_R 18
#define BACKSPACE 127
#define TAB 9
#define ENTER 10
#define DIR_LENGTH 1000

#define MAXIMUM_CACHE_HIST 1000

const string HIST_FILE_NAME = ".history";
deque<string> history;

void displayPromptToScreen() {
    char buf[DIR_LENGTH];
    getcwd(buf, DIR_LENGTH);
    string dir(buf,buf+strlen(buf));
    dir = dir.substr(dir.find_last_of("/") + 1);
    cout << COLOR_GREEN << "bash: " << COLOR_BLUE << dir << COLOR_RESET << "$ ";
}
void printHistory(int size) {
    deque<string> temp = history;
    while(size-- && !temp.empty()) {
        cout << temp.front() << endl;
        temp.pop_front();
    }
}
string readCommand() {
    string s;
    getline(cin,s);
    cout << "[" << s << "]" << endl;
    if(s.compare("exit")==0)
        return s;
    return s;
}

signed main()
{
    // Loading the History of Commands from the File
    history.clear();
    ifstream file(HIST_FILE_NAME);
    if(!file.is_open()) {
        cout << COLOR_RED << "\nWARN:: " << COLOR_RESET << "Could NOT Load History of Commands...." << endl;
        cout << ".... " << COLOR_RED << "Error:: " << COLOR_RESET << "History File Absent in the given System Path....." << endl;
    }
    else {
        string cmd = "";
        while(getline(file, cmd) && history.size() <= MAXIMUM_CACHE_HIST) history.push_back(cmd);
    }
    if(file.is_open()) file.close();
    while(1) {
        displayPromptToScreen();
        string command = readCommand();
        if(command.size()==0){
            continue;
        }
        // Remove Whitespaces from beginning and back of string command
        int i = 0;
        while(i < command.size() && command[i]==' ') i++;
        command = command.substr(i);
        while(command.size() && command.back()==' ') command.pop_back();
        if(command.size()==0){
            continue;
        }
        if(history.size() == MAXIMUM_CACHE_HIST) history.pop_front();
        history.push_back(command);
        if(history.size() > MAXIMUM_CACHE_HIST) history.pop_front();
        if(command == "exit") 
            break;
        else if(command.substr(0,7)=="history") {
            try {
                int size = stoi(command.substr(8));
                // int size = 10;
                printHistory(size);
            }
            catch(...) {
                cout << "Error" << endl;
            }
        }
    }
    // Saving the history to the file 
    ofstream oufile(HIST_FILE_NAME);
    if(!oufile.is_open()) {
        cout << COLOR_RED << "\nWARN:: " << COLOR_RESET << "Could NOT Find History of Commands...." << endl;
        cout << ".... " << COLOR_RED << "Error:: " << COLOR_RESET << "History File Absent in the given System Path....." << endl;
    }
    else {
        for(auto it: history) {
            string temp = it;
            oufile << temp << endl;
        }
    }
    if(oufile.is_open()) oufile.close();
    return 0;
}