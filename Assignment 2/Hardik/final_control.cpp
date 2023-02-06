/*
The ASCII value sequence for the left arrow key in Mac OS is: 27, 91, 68.
The ASCII value sequence for the right arrow key in Mac OS is: 27, 91, 67.
*/

#include <bits/stdc++.h>
#include <ncurses.h>
#include <string>
using namespace std;

// deque<string> history;
// const string HIST_FILE = ".history";


int ascii(char ch)
{
    return (int)ch;
}
// void load_history()
// {
//     history.clear();
//     ifstream file(HIST_FILE);
//     if (!file.is_open()) 
//         return;
//     else {
//         string line = "";
//         while (getline(file, line)) {
//             history.push_back(line);
//         }
//     }
//     if(file.is_open())
//         file.close();
// }
// void print_history()
// {
//     int i = 1;
//     for(auto it: history) 
//     {
//         printw("%d %s\n", i++, it.c_str());
//         // cout << i++ << " " << it << "\n";
//     }
//     printw("\n");
// }
int main()
{
    
    initscr();
    noecho();
    cbreak();
    // load_history();
    // Uncomment this line to see the previous history saved
    // print_history(); 
    string input;
    int cursorPos = 0;
    int line  = 0;
    while (1)
    {
        clrtoeol();
        mvprintw(line, 0, "Enter your Command: ");
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
            // Left Arrow Key
            else if (c == 27)
            {
                c = getch();
                if (c == 91)
                {
                    c = getch();
                    if (c == 68)
                    {
                        if (cursorPos > 0)
                            cursorPos--;
                    }
                    else if (c == 67)
                    {
                        if (cursorPos < input.length())
                            cursorPos++;
                    }
                }
            }
            else
            {
                input.insert(cursorPos, 1, char(c));
                cursorPos++;
            }
            mvprintw(line,20, input.c_str());
            clrtoeol();
            move(line, 20 + cursorPos);
            refresh();
        }
        line++;
        mvprintw(line, 0, "Your entered : %s\n", input.c_str());
        line++;
        // history.push_back(input);
        // print_history();
        input.clear();
        cursorPos = 0;
    }
    endwin();
    return 0;
}
