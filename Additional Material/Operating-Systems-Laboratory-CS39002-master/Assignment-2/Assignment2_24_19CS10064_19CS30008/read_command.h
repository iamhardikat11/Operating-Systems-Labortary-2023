#ifndef __READ_COMMAND_H
#define __READ_COMMAND_H

#include <string>

using namespace std;

#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN "\033[1;36m"
#define COLOR_RESET "\033[0m"

#define DIR_LENGTH 1000

#define CTRL_CZ -1
#define CTRL_D 4
#define CTRL_R 18
#define BACKSPACE 127
#define TAB 9
#define ENTER 10

void displayPrompt();
string readCommand();

#endif