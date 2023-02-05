#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAX_INPUT_LENGTH 100

// Function to get the input from the user
char *get_input() {
  static int pos = 0;
  static char *history[10];
  int c, i = 0;
  char *input = (char *)malloc(MAX_INPUT_LENGTH * sizeof(char));

  while ((c = getchar()) != '\n' && c != EOF) {
    // Handle up arrow key
    if (c == '\033') {
      getchar();
      getchar();
      if (pos > 0) {
        // Clear the current line
        for (i = 0; i < strlen(input); i++) {
          printf("\b \b");
        }
        // Decrement the position
        pos--;
        // Copy the previous command to input
        strcpy(input, history[pos]);
        // Print the previous command
        printf("%s", input);
      }
      continue;
    }
    // Handle down arrow key
    if (c == '\033') {
      getchar();
      getchar();
      if (pos < 9 && history[pos + 1] != NULL) {
        // Clear the current line
        for (i = 0; i < strlen(input); i++) {
          printf("\b \b");
        }
        // Increment the position
        pos++;
        // Copy the next command to input
        strcpy(input, history[pos]);
        // Print the next command
        printf("%s", input);
      }
      continue;
    }
    // Handle normal input
    input[i++] = c;
    printf("%c", c);
  }
  input[i] = '\0';
  // Save the input to history
  history[pos++ % 10] = input;
  return input;
}

int main() {
  char *input;

  // Get input from the user
  input = get_input();

  // Use the input
  printf("\nYou entered: %s\n", input);

  // Free the memory used by input
  free(input);

  return 0;
}
