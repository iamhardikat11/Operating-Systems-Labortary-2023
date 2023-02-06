#include <bits/stdc++.h>
#include <ncurses.h>
#include <string>
using namespace std;

int main()
{
    initscr();
    noecho();
    cbreak();

    string input;
    int cursorPos = 0;
    int line = 0;

    while (1)
    {
        mvprintw(line, 0, "Enter your Command: ");
        refresh();
        int c;
        while ((c = getch()) != 10)
        {
            if (c == 49)
                cursorPos = 0;
            else if (c == 57)
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
            mvprintw(line, 20, input.c_str());
            move(line, 20 + cursorPos);
            refresh();
        }
        line++;
        mvprintw(line, 0, "You entered: %s\n", input.c_str());
        line++;
        input.clear();
        cursorPos = 0;
    }

    endwin();
    return 0;
}
