<<<<<<< HEAD
#include <bits/stdc++.h>
#include <termios.h>
#include <unistd.h>
// #include <conio.h> // for _getch() function
using namespace std;

=======
// #include <bits/stdc++.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
>>>>>>> 9bdeee6dd0d50f14f3addb0da501391967de2bf4
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
<<<<<<< HEAD
int main() {
  std::string input;
  while (true) {
    char c = getch();
    // if (c == 1) { // ctrl + 1
    //   std::cout << "Ctrl+1 was pressed. Moving cursor to the start of the line." << std::endl;
    // } else if (c == 5) { //ctrl + 9
    //   std::cout << "Ctrl+9 was pressed. Moving cursor to the end of the line." << std::endl;
    // } else 
    {
      std::cout << "You entered: " << c << std::endl;
    }
  }
  return 0;
}
=======
int main()
{
    while(true)
    {
        int ch = getch();
        printf("You Entered: %d\n", ch);        
    }
    return 0;
}
>>>>>>> 9bdeee6dd0d50f14f3addb0da501391967de2bf4
