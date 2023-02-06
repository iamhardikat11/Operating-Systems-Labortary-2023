// #include <iostream>
// #include <vector>
// #include <string>
// #include <ncurses.h>
// #include <termios.h>
// #include <unistd.h>
// #include <fcntl.h>

// using namespace std;

// int main() {
//   // Initialize ncurses
//   initscr();
//   cbreak();
//   noecho();
//   nonl();
//   intrflush(stdscr, false);
//   keypad(stdscr, true);

//   // Store history of commands
//   vector<string> history;
//   int current = -1;

//   // Get the terminal attributes
//   struct termios ttystate;
//   tcgetattr(STDIN_FILENO, &ttystate);

//   // Turn off canonical mode
//   ttystate.c_lflag &= ~ICANON;
//   ttystate.c_cc[VMIN] = 1;
//   tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

//   // Loop to read input
//   while (true) {
//     // Print prompt
//     printw("> ");
//     string line;
//     int ch;
//     while ((ch = getch()) != '\n') {
//       if (ch == KEY_UP) {
//         // Move up in history
//         if (current > 0) {
//           current--;
//           line = history[current];
//           // Clear line
//           move(getcury(stdscr), 2);
//           clrtoeol();
//           // Print line
//           printw("%s", line.c_str());
//         }
//       } else if (ch == KEY_DOWN) {
//         // Move down in history
//         if (current < history.size() - 1) {
//           current++;
//           line = history[current];
//           // Clear line
//           move(getcury(stdscr), 2);
//           clrtoeol();
//           // Print line
//           printw("%s", line.c_str());
//         }
//       } else if (ch == 1) {
//         // Move cursor to the beginning of the line
//         move(getcury(stdscr), 2);
//       } else if (ch == 9) {
//         // Move cursor to the end of the line
//         move(getcury(stdscr), line.length() + 2);
//       } else {
//         // Add the character to the line
//         line += ch;
//         addch(ch);
//       }
//     }

//     // Add the line to history
//     history.push_back(line);
//     current = history.size();

//     // Detect simple malware
//     if (line == "sb") {
//       printw("Simple malware detected.");
//     } else {
//       printw("Unknown command.");
//     }

//     // Move to next line
//     move(getcury(stdscr) + 1, 0);
//   }

//   // End ncurses
//   endwin();

//   return 0;
// }
