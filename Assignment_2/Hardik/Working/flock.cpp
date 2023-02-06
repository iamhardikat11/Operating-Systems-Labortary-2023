#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/file.h>

int main() {
  std::ofstream file("filename.txt", std::ios::out | std::ios::app);
  
  while (1) {
    int fd = open("filename.txt", O_WRONLY);
    // int fd = fileno((FILE *)file);
    flock(fd, LOCK_EX);
    file << "Hi\n";
    // flock(fd, LOCK_UN);
    // file.flush();
    sleep(5);
  }

  return 0;
}
