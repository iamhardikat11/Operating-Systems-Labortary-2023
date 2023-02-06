#include <bits/stdc++.h>
// For Linux user
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/file.h>
// For windows user
// #include <windows.h>

using namespace std;
void childProcess(char *file_path)
{
    int fd = open(file_path, O_WRONLY);

    if (fd == -1)
    {
        perror("Child: Error opening file");
        return;
    }

    // lock the file for writing
    if (flock(fd, LOCK_EX) == -1)
    {
        perror("Child: Error locking file");
        close(fd);
        return;
    }
    const char message[] = "Hello World\n";
    int n;
    while (n = write(fd, message, sizeof(message)))
    {
        if (n == -1)
        {
            perror("Child: Error writing to file");
            return;
        }
        sleep(2);
    }
    // close(fd);
}
int main()
{
    char *file_path = (char *)malloc(100 * sizeof(char));
    cout << "Enter the File's Path: ";
    cin >> file_path;
    // pid_t pid = fork();
    // if (pid == -1)
    // {
    //     perror("Error creating child process");
    //     return 1;
    // }
    // else if (pid == 0)
    {
        childProcess(file_path);
        return 0;
    }
    // else
    // {
    //     int status;
    //     waitpid(pid, &status, 0);
    //     cout << "Child process finished" << endl;
    // }
    return 0;
}