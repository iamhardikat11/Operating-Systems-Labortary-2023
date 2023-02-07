#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sstream>

std::string get_process_pid_with_lock(const char *filepath)
{
    int fd = open(filepath, O_RDONLY);
    if (fd == -1)
    {
        std::cerr << "Error opening file: " << filepath << std::endl;
        return "";
    }

    // get file lock info
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    if (fcntl(fd, F_GETLK, &lock) == -1)
    {
        std::cerr << "Error getting lock information for file: " << filepath << std::endl;
        close(fd);
        return "";
    }

    close(fd);

    // return process ID if file is locked, empty string otherwise
    std::ostringstream oss;
    if (lock.l_type != F_UNLCK)
    {
        oss << lock.l_pid;
    }
    return oss.str();
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filepath>" << std::endl;
        return 1;
    }
    std::cout << get_process_pid_with_lock(argv[1]) << std::endl;
    return 0;
}
