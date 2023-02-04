#include <bits/stdc++.h>
#include <termios.h>
#include <unistd.h>

using namespace std;
#define MAX_LENGTH 100

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

void print_(deque<string> history)
{
  for(auto it: history) cout << it << " ";
  cout << endl;
}
int main()
{
  // char **history = (char **)malloc(MAX_LENGTH * sizeof(char *));
  deque<string> history;
  int hist_cur = 0;
  int flag1 = 0;
  int flag2 = 0;
  while (1)
  {
    string input;
    int i, ch;
    int flag = 0;
    if(!flag1)
      cout << "Enter your command: ";
    while ((ch = getch()) != '\n')
    {
      if (ch == 27)
      {
        // Arrow key
        ch = getch();
        if (ch == 91)
        {
          ch = getch();
          if (ch == 65)
          {
            // Up arrow
            if (hist_cur >= 0 && hist_cur < history.size())
            {
              flag = 1;
              flag1 = 1;
              cout << "\r" << "\033[K" << "Enter your Command: ";
              cout << history[hist_cur];
              hist_cur--;
              if(hist_cur == -1) hist_cur++;
              flag2 = 1;
              break;
            }
          }
          else if (ch == 66)
          {
            // Down arrow
            if (hist_cur < history.size() && hist_cur >= 0)
            {
              flag = 1;
              flag1 = 1;
              cout << "\r" <<  "\033[K" << "Enter your Command: " << history[hist_cur];
              hist_cur++;
              break;
            }
          }
        }
      }
      else if (ch == 127 || ch == 8)
      {
        // Backspace
        if (input.size() > 0)
        {
          input.pop_back();
          printf("\b \b");
        }
      }
      else {
          input.push_back((char)ch);
          putchar(ch);
      }
    }
    if(!flag) {
      cout << endl;
      history.push_back(input);
      hist_cur = history.size()-1;
      cout << "You Entered: " << input << endl;
      print_(history);
      flag1 = 0;
      flag2 = 0;
    }
  }
  return 0;
}