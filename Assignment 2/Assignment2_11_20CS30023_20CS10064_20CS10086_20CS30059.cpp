#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <unistd.h>
#include <algorithm>

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


void displayPromptToScreen() {
    char buf[DIR_LENGTH];
    getcwd(buf, DIR_LENGTH);
    string dir(buf,buf+strlen(buf));
    dir = dir.substr(dir.find_last_of("/") + 1);
    cout << COLOR_GREEN << "bash: " << COLOR_BLUE << dir << COLOR_RESET << "$ ";
}
string readCommand() {
    string s;
    getline(cin,s);
    cout << "[" << s << "]" << endl;
    if(s.compare("exit")==0)
        exit(0);
    
    // else if()
    // else {
    //     return 1;
    // }
    return s;
}

signed main()
{
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
        cout << "P" << endl;
    }
    return 0;
}