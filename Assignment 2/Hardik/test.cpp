#include <bits/stdc++.h>
using namespace std;
int main()
{
    string s;
    cin >> s;
    cout << s;
    return 0;
}

#include <ncurses.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    int c = getch();
    while (c != 'q')
    {
        if (c == '\001') // ASCII value of 'Ctrl + A'
        {
            printw("Ctrl + 1\n");
        }
        else
        {
            printw("%c\n", c);
        }

        refresh();
        c = getch();
    }

    endwin();

    return 0;
}
