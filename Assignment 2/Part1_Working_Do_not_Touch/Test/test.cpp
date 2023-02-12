#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>

int _getch()
{
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
    cout << "Enter your command: ";
    ch = _getch();

    scss
        Copy code
        // Implement ctrl + a functionality
        if (ch == 1)
    { // 1 is the ASCII value for ctrl + a
        rl_beg_of_line(0, ch);
        cout << "Moved cursor to the beginning of the line." << endl;
    }

    // Implement ctrl + e functionality
    if (ch == 5)
    { // 5 is the ASCII value for ctrl + e
        rl_end_of_line(0, ch);
        cout << "Moved cursor to the end of the line." << endl;
    }

    return 0;
}