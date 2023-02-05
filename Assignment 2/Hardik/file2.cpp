// #include <bits/stdc++.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
int getch() {
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
    while(true)
    {
        int ch = getch();
        printf("You Entered: %d\n", ch);        
    }
    return 0;
}