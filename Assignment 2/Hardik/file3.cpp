#include <ncurses.h>
#include <bits/stdc++.h>
#include <termios.h>
#include <unistd.h>
using namespace std;

int _getch() {
  struct termios old_t, new_t;
  int ch;
  tcgetattr(STDIN_FILENO, &old_t);
  new_t = old_t;
  new_t.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_t);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
  return ch;
}
int main()
{
    int ch;
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    
    while (1)
    {
        printf("Enter your Command: ");
        string input;
        while((ch = _getch()) != '\n'){
            if (ch == BUTTON_CTRL && ((ch = _getch()) == 49))
            {
                putchar(49);
                // printf("1\n");
                // move(0, 0);
                break;
            }
            else if (ch == BUTTON_CTRL && ((ch =_getch()) == 57))
            {
                putchar(57);
                // int x, y;
                // getmaxyx(stdscr, y, x);
                // move(y - 1, x - 1);
                break;
            }
            else {
                input.push_back(ch);
                putchar(ch);
            }
        }
        printf("1\n");
    }
    endwin();
    return 0;
}
