/*
The ASCII value sequence for the left arrow key in Mac OS is: 27, 91, 68.
The ASCII value sequence for the right arrow key in Mac OS is: 27, 91, 67.
*/

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
        printw("Enter your Command: ");
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
            clear();
            refresh();
            printw("Enter your Command: ");
            printw(input.c_str());
            move(0, 20 + cursorPos);
            refresh();
        }
        printw("\nYour entered command: %s\n", input.c_str());
        input.clear();
        cursorPos = 0;
    }
    endwin();
    return 0;
}
