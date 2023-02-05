#include <ncurses.h>
#include <stdio.h>
int main()
{
    int ch;
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    while (1)
    {
        ch = getch();

        if (ch == 49 && (ch == KEY_CTRL_L || ch == KEY_CTRL_R))
        {
            printf("1\n");
            move(0, 0);
        }
        else if (ch == 57 && (ch == KEY_CTRL_L || ch == KEY_CTRL_R))
        {
            printf("9\n");
            int x, y;
            getmaxyx(stdscr, y, x);
            move(y - 1, x - 1);
        }
    }
    endwin();
    return 0;
}