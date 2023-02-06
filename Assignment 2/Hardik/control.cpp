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
    int row = 0;
    while (1)
    {
        move(row, 0);
        clrtoeol();
        printw("Enter your Command: ");
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
            move(row, 0);
            clrtoeol();
            printw("Enter your Command: ");
            printw(input.c_str());
            move(row, 20 + cursorPos);
            refresh();
        }
        row++;
        move(row, 0);
        clrtoeol();
        printw("Your entered command: %s\n", input.c_str());
        printw(input.c_str());
        move(row, 20 + cursorPos);
        refresh();
        // refresh();
        // getch();
        input.clear();
        cursorPos = 0;
    }
    endwin();
    return 0;
}
