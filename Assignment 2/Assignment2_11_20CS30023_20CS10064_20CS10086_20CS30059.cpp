#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
// #include <termio.h>
#include <unistd.h>

#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN "\033[1;36m"
#define COLOR_RESET "\033[0m"

#define CTRL_CZ -1
#define CTRL_D 4
#define CTRL_R 18
#define BACKSPACE 127
#define TAB 9
#define ENTER 10
#define DIR_LENGTH 1000

char* history[1000];


void displayPromptToScreen()
{
    char buf[DIR_LENGTH];
    getcwd(buf, DIR_LENGTH);
    int len = strlen(buf);
    char *dir = (char *)malloc(len + 1);
    strcpy(dir, buf);
    char *p = strrchr(dir, '/');
    if (p)
    {
        p++;
        *p = '\0';
    }
    printf("%sbash: %s%s%s$ ", COLOR_GREEN, COLOR_BLUE, dir, COLOR_RESET);
    free(dir);
}
void readCommand(char *s)
{
    fgets(s, DIR_LENGTH, stdin);
    if (strcmp(s, "exit\n") == 0)
        exit(0);
}

signed main()
{
    //load
    while (1)
    {
        char *str = (char *)malloc(1000 * sizeof(char));
        displayPromptToScreen();
        readCommand(str);
        if(strcmp(str, "\n") == 0) continue;
        if (strlen(str) == 0) continue;
        str[strlen(str) - 1] = '\0';
        // Remove Whitespaces from beginning and back of string command
        char *command = (char *)malloc(1000 * sizeof(char));
        int i = 0, j = 0;
        while (i < strlen(str) && str[i] == ' ') i++;
        if (strlen(str) == 0) continue;
        while (i < strlen(str) && j < 1000) command[j++] = str[i++];
        i = strlen(command) - 1;
        while (i >= 0 && command[i] == ' ') command[i--] = '\0';
        // addToHistory(command);
        free(str);
        free(command);
    }
    return 0;
}