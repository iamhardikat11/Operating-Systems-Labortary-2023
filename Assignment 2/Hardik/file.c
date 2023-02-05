#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
int main()
{
  char *input;
  while (1)
  {
    input = readline("$ "); // read input from the terminal
    if (input == NULL) // exit if EOF is reached
      break;
    add_history(input); // add input to the history
    printf("You entered: %s\n", input);
    free(input);
  }
  return 0;
}
