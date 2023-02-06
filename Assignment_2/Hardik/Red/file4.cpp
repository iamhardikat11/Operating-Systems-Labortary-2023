/*
    In a shell often after writing a long command you need to go back to the beginning of the
    command. In your shell add a feature "ctrl + 1" will bring the cursor to the beginning of
    your typed command, and a feature "ctrl + 9" that will bring the cursor at the end of the
    typed command.
*/
#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
int main()
{
  int ch;

  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();

  printw("Press 'Ctrl + 1' or 'Ctrl + A' to quit\n");
  refresh();

  while((ch = getch()) != '\001')
  {
    printw("1::%c\n", ch);  
    switch(ch)
    {
      case '1':
        if(getch() == '1') {
          printw("2::%c\n", ch);  
          goto end;
        }
        break;
    }
  }
end:
  sleep(10);
  endwin();
  return 0;
}
