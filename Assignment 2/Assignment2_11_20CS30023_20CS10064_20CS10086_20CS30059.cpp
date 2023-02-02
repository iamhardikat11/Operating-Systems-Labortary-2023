#include <bits/stdc++.h>
using namespace std;

#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN "\033[1;36m"
#define COLOR_RESET "\033[0m"

#define DIR_LENGTH 1000


void displayPrompt() {
    char buf[DIR_LENGTH];
    getcwd(buf, DIR_LENGTH);
    string dir(buf);
    dir = dir.substr(dir.find_last_of("/") + 1);
    cout << COLOR_GREEN << "bash: " << COLOR_BLUE << dir << COLOR_RESET << "$ ";
}
int readCommand() {
    string s;
    getline(cin,s);
    cout << "[" << s << "]" << endl;
    if(s.compare("exit")==0) {
        return 0; 
    }
    else if()
    else {
        return 1;
    }
}

signed main()
{
    cout << "Hello" << endl;
    
    while(1) {
        displayPrompt();
        int k = readCommand();
        if(k==0) {
            break;
        }
    }
    return 0;
}