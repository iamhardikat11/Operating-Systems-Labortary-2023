#include <iostream>
#include <termios.h>
#include <signal.h>
#include <unistd.h>

struct termios terminal_settings;
bool is_ctrl_1_pressed = false;

void signal_handler(int signal_number) {
  if (signal_number == SIGINT) {
    is_ctrl_1_pressed = true;
  }
}

int main() {
  struct sigaction signal_action;
  sigemptyset(&signal_action.sa_mask);
  signal_action.sa_handler = signal_handler;
  sigaction(SIGINT, &signal_action, NULL);

  tcgetattr(STDIN_FILENO, &terminal_settings);
  terminal_settings.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &terminal_settings);

  while (!is_ctrl_1_pressed) {
    std::cout << "Press Ctrl+1 to exit." << std::endl;
    sleep(1);
  }

  terminal_settings.c_lflag |= (ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &terminal_settings);

  std::cout << "Ctrl+1 was pressed." << std::endl;
  return 0;
}
