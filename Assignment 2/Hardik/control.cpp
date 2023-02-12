// // #include <bits/stdc++.h>
// // #include <ncurses.h>
// // #include <string>
// // using namespace std;

// // int ascii(char ch)
// // {
// //     return (int)ch;
// // }

// // int main()
// // {
// //     initscr();
// //     noecho();
// //     cbreak();

// //     string input;
// //     int cursorPos = 0;
// //     int row = 0;
// //     while (1)
// //     {
// //         move(row, 0);
// //         clrtoeol();
// //         printw("Enter your Command: ");
// //         refresh();
// //         int c;
// //         while ((c = getch()) != 10)
// //         { 
// //             if (c == ascii('1'))
// //                 cursorPos = 0;
// //             else if (c == ascii('9'))
// //                 cursorPos = input.length();
// //             else if (c == KEY_BACKSPACE || c == 127)
// //             {
// //                 if (cursorPos > 0)
// //                 {
// //                     input.erase(cursorPos - 1, 1);
// //                     cursorPos--;
// //                 }
// //             }
// //             else
// //             {
// //                 input.insert(cursorPos, 1, char(c));
// //                 cursorPos++;
// //             }
// //             move(row, 0);
// //             clrtoeol();
// //             printw("Enter your Command: ");
// //             printw(input.c_str());
// //             move(row, 20 + cursorPos);
// //             refresh();
// //         }
// //         row++;
// //         move(row, 0);
// //         clrtoeol();
// //         printw("Your entered command: %s\n", input.c_str());
// //         printw(input.c_str());
// //         move(row, 20 + cursorPos);
// //         refresh();
// //         // refresh();
// //         // getch();
// //         input.clear();
// //         cursorPos = 0;
// //     }
// //     endwin();
// //     return 0;
// // }
// #include <ncurses.h>
// #include <unistd.h>

// // int main() {
// //     initscr(); // Initialize the ncurses library
// //     noecho(); // Don't echo typed characters to the screen
// //     cbreak(); // Disable buffered input

// //     int y, x;
// //     getmaxyx(stdscr, y, x); // Get the size of the terminal screen
// //     int i=0;
// //     while(1) {
// //         move(i++, 0); // Move the cursor to the specified position
// //         printw("This is some text that will scroll up the screen."); // Print text to the screen
// //         refresh(); // Refresh the screen
// //         usleep(100000); // Sleep for 100 milliseconds
// //     }

// //     endwin(); // Deinitialize the ncurses library
// //     return 0;
// // }

// #include <ncurses.h>
// #include <string>

// int main() {
//     initscr(); // Initialize the ncurses library
//     noecho(); // Don't echo typed characters to the screen
//     cbreak(); // Disable buffered input
//     scrollok(stdscr, true); // Enable scrolling for the main window

//     int y, x;
//     getmaxyx(stdscr, y, x); // Get the size of the terminal screen

//     for (int i = 0; i < y; ++i) {
//         move(i, 0); // Move the cursor to the specified position
//         addstr("Enter your Command: "); // Print the prompt
//         refresh(); // Refresh the screen
//         napms(100); // Sleep for 100 milliseconds
//         scroll(stdscr); // Scroll the screen down by one line
//     }

//     endwin(); // Deinitialize the ncurses library
//     return 0;
// }
#include <termios.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

bool ctrl_one_pressed = false;
void signal_handler(int signum) {
  if (signum == SIGINT) {
    ctrl_one_pressed = true;
  }
}
int main() {
  struct termios old_tio, new_tio;
  tcgetattr(STDIN_FILENO, &old_tio);
  new_tio = old_tio;
  new_tio.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
  signal(SIGINT, signal_handler);
  while (1) {
    char c = getchar();
    if (ctrl_one_pressed) {
      printf("ctrl + 1 pressed\n");
      break;
    }
    printf("%c\n", c);
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
  return 0;
}
