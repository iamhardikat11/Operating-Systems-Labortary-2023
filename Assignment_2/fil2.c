#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

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

int main() {
  while(1) {  
  char input[MAX_LENGTH];
  int length = 0, i, c;

  printf("Enter your command: ");
  while ((c = getch()) != '\n') {
    if (c == 27) {
      c = getch();
      if (c == 91) {
        c = getch();
        switch (c) {
          case 65: // Up arrow
            printf("\r");
            printf("Up");
            break;
          case 66: // Down arrow
            printf("\r");
            printf("Down");
            break;
        }
      }
    } else {
      if (c == 127 || c == 8) {
        if (length > 0) {
          input[--length] = '\0';
          printf("\b \b");
        }
      } else {
        input[length++] = c;
        putchar(c);
      }
    }
  }

  input[length] = '\0';
  printf("\nYou entered: %s\n", input);
  }
  return 0;
}
