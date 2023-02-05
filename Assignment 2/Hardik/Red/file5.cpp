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
    std::cout << "shjdlkf" << std::endl;
    while (1)
    {
        printw("Enter your Command: ");
        refresh();
        int c = getch();
        while (c != 27)
        { // 27 is the ASCII code for escape key
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
            printw("Enter your Command: ");
            printw(input.c_str());
            move(0, 20 + cursorPos);
            refresh();
            c = getch();
        }
        printw("\nYou entered: %s\n", input.c_str());
    }
    endwin();
    return 0;
}
