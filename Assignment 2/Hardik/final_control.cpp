/*
Steps to Run the Code:-
    g++ -o fc final_control.cpp -lncurses
*/
#include <bits/stdc++.h>
#include <ncurses.h>
#include <unistd.h>
#include <ncurses.h>
#include <string>
using namespace std;

string C_RED     =   "\033[1;31m";
string C_GREEN   =   "\033[1;32m";
string C_YELLOW  =   "\033[1;33m";
string C_BLUE    =   "\033[1;34m";
string C_MAGENTA =   "\033[1;35m";
string C_CYAN    =   "\033[1;36m";
string C_RESET  =   "\033[0m";
#define DIR_LENGTH 1000
#define HIST_SIZE 1000


deque<string> history;
const string HIST_FILE = ".history";
int line  = 0;

// void displayPromptToScreen()
// {
//     char buf[DIR_LENGTH];
//     getcwd(buf, DIR_LENGTH);
//     int len = strlen(buf);
//     char *dir = (char *)malloc(len + 1);
//     strcpy(dir, buf);
//     char *p = strrchr(dir, '/');
//     if (p)
//     {
//         p++;
//         *p = '\0';
//     }
//     // mvprintw(line, 0, "%sbash: %s%s%s$ ", C_GREEN.c_str(), C_BLUE.c_str(), dir, C_RESET.c_str());
//     attron(COLOR_PAIR(1));
//     mvprintw(line, 0, "This is red text");
//     attroff(COLOR_PAIR(1));

//     free(dir);
// }

int ascii(char ch)
{
    return (int)ch;
}
void load_history()
{
    history.clear();
    ifstream file(HIST_FILE);
    if (!file.is_open()) 
        return;
    else {
        string line = "";
        while (getline(file, line)) 
            history.push_back(line);
    }
    if(file.is_open())
        file.close();
}
// Print History
void print_history()
{
    int i = 1;
    for(auto it: history) 
        mvprintw(line++, 0, "%d %s\n", i++, it.c_str());
    mvprintw(line, 0, "\n");
}
// Add a command to the history
void addToHistory(string s) {
    if (history.size() == HIST_SIZE)
        history.pop_front();
    history.push_back(s);
}

// Save the history to the file when the shell exits
void updateHistory() {
    ofstream file(HIST_FILE);
    if (!file.is_open())
        return;
    else {
        for (auto it : history) {
            string temp = it + "\n";
            file << temp;
        }
    }
    if(file.is_open())
        file.close();
}
int main()
{
    
    initscr();
    noecho();
    cbreak();
    // scrollok(stdscr, true);
    scrollok(stdscr, TRUE);  //These lines are the ones I think are causing issues
    idlok(stdscr, TRUE);     //<<<
    keypad(stdscr, TRUE);    //<<<
    load_history();
    // Uncomment this line to see the previous history saved
    print_history(); 
    string input;
    int cursorPos = 0;
    const char* prompt = "Enter your Command: ";
    while (1)
    {
        clrtoeol();
        // displayPromptToScreen();
        mvprintw(line, 0, prompt);
        int hist_cur = history.size()-1;
        refresh();
        int c;
        while ((c = getch()) != 10)
        {
            // Ctrl + 1
            if (c == ascii('1'))
                cursorPos = 0;
            // Ctrl + 9
            else if (c == ascii('9'))
                cursorPos = input.length();
            // BackSpace
            else if (c == KEY_BACKSPACE || c == 127)
            {
                if (cursorPos > 0)
                {
                    input.erase(cursorPos - 1, 1);
                    cursorPos--;
                }
            }
            // Arrow Keys
            else if (c == 27)
            {
                c = getch();
                if (c == 91)
                {
                    c = getch();
                    // Left Arrow Key
                    if (c == 68)
                    {
                        if (cursorPos > 0)
                            cursorPos--;
                    }
                    // RIght Arrow Key
                    else if (c == 67)
                    {
                        if (cursorPos < input.length())
                            cursorPos++;
                    }
                    else if (c == 65)
                    {
                        if(hist_cur >= 0) {
                            input = history[hist_cur];
                            hist_cur--;
                            if(hist_cur == -1) hist_cur = 0;
                            cursorPos = input.length();
                        }
                    }
                    else if (c == 66)
                    {
                        if(hist_cur < history.size()) {
                            input = history[hist_cur];
                            hist_cur++;
                            if(hist_cur == history.size()) hist_cur = history.size()-1;
                            cursorPos = input.length();
                        }
                    }
                    else
                    {
                        ;
                    }
                }
            }
            else
            {
                input.insert(cursorPos, 1, char(c));
                cursorPos++;
            }
            mvprintw(line,strlen(prompt), input.c_str());
            clrtoeol();
            move(line, strlen(prompt) + cursorPos);
            refresh();
        }
        line++;
        mvprintw(line, 0, "Your entered : %s\n", input.c_str());
        line++;
        addToHistory(input);
        string ans = "\033[F";
        mvprintw(line, 0, ans.c_str());
        // std::cout << "\033[F";
        if(input == "exit") {
            break;
        }
        else if(input == "pwd") {
            char *pwd = (char *)malloc(DIR_LENGTH*sizeof(char));
            getcwd(pwd, DIR_LENGTH);
            // line++;
            mvprintw(line, 0, pwd);
            line++;
        }
        input.clear();
        cursorPos = 0;
    }
    endwin();
    updateHistory();
    return 0;
}