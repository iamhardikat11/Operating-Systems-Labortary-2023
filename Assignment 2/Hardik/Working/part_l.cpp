/*
Some processes start reading/writing a file after locking (using flock) and never release
it, making it impossible to delete a file. Implement a command "delep" (short for delete
with extreme prejudice) which takes a filepath as argument and spawns a child process.
The child process will help to list all the process pids which have this file open as well as
all the process pids which are holding a lock over the file. Then the parent process will
show it to the user and ask permission to kill each of those processes using a yes/no
prompt. On putting yes, all of those processes will be killed using signal and then the file
will be deleted. Also create a test code which will spawn a process which locks a file and
try to write to a file using a while 1 loop.
*/
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/file.h>

using namespace std;

void childProcess(const char* filename)
{
    int fd = open(filename, O_WRONLY);

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
    char* msg = "Hi";
    while (1)
    {
        // write to the file
        if (write(fd, &msg, sizeof(msg)) == -1)
        {
            perror("Child: Error writing to file");
            break;
        }
        sleep(10);
    }
    // unlock the file
    if (flock(fd, LOCK_UN) == -1)
    {
        perror("Child: Error unlocking file");
    }
    close(fd);
}

int main()
{
    char* file_path = (char *)malloc(100*sizeof(char));
    cout << "Enter the File's Path: ";
    cin >> file_path;
    cout << endl;
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Error creating child process");
        return 1;
    }
    else if (pid == 0)
    {
        childProcess(file_path);
        return 0;
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        cout << "Child process finished" << endl;
    }
    return 0;
}
