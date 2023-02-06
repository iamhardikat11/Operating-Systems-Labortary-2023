#include <bits/stdc++.h>
#include <ncurses.h>
#include <string>
using namespace std;

int ascii(char ch)
{
    return (int)ch;
}

int main()
{
    initscr();
    noecho();
    cbreak();

    string input;
    int cursorPos = 0;

    while (1)
    {
        clear();
        printw("Enter your Command:\n");
        refresh();
        int c;
        while ((c = getch()) != 10)
        { 
            if (c == ascii('1'))
                cursorPos = 0;
            else if (c == ascii('9'))
                cursorPos = input.length();
            else if (c == KEY_BACKSPACE || c == 127)
            {
                if (cursorPos > 0)
                {
                    input.erase(cursorPos - 1, 1);
                    cursorPos--;
                }
            }
            else
            {
                input.insert(cursorPos, 1, char(c));
                cursorPos++;
            }
            clear();
            printw("Enter your Command:\n");
            printw(input.c_str());
            move(1, 20 + cursorPos);
            refresh();
        }
        printw("\nYou entered: %s\n", input.c_str());
        // getch();
        clear();
    }
    endwin();
    return 0;
}
