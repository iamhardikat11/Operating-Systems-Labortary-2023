#include "multiWatch.h"

#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ShellException.h"
#include "utility.h"

using namespace std;

int inotify_fd;           // The inotify instance to monitor the file descriptors
map<pid_t, int> pgid_wd;  // Map process group id to watch descriptor
map<int, int> wd_ind;     // Map watch descriptor to index in the vector

// Creates pipelines for all commands inside quotes for multiWatch
vector<Pipeline*> parseMultiWatch(string cmd, string& output_file) {
    trim(cmd);
    if (cmd.length() < 10) {  // Not multiWatch
        return vector<Pipeline*>();
    }
    vector<Pipeline*> pipelines;
    if (cmd.substr(0, 10) == "multiWatch") {
        cmd = cmd.substr(10);
        trim(cmd);
        if (cmd.length()) {
            if (cmd.back() != ']') {
                int i = cmd.length() - 1;
                while (i >= 0 && cmd[i] != ']') {
                    i--;
                }
                if (i == -1) {
                    throw ShellException("Could not find ']");
                } else {
                    // Extract output filename if present
                    output_file = cmd.substr(i + 1);
                    cmd = cmd.substr(0, i + 1);
                    trim(output_file);
                    if (output_file.length()) {
                        if (output_file[0] == '>') {
                            output_file = output_file.substr(1);
                            trim(output_file);
                            if (output_file.length() == 0) {
                                throw ShellException("No output file after >");
                            }
                        } else {
                            throw ShellException("Unable to parse after multiWatch");
                        }
                    }
                }
            }
            trim(cmd);
            cmd.pop_back();
            if (cmd[0] == '[') {
                cmd = cmd.substr(1);
                vector<string> commands = split(cmd, ',');
                for (auto& command : commands) {
                    trim(command);
                    if (command[0] == '\"' && command.back() == '\"') {
                        command = command.substr(1, command.length() - 2);
                    } else {
                        throw ShellException("No quotes around command");
                    }
                }
                if (commands.empty()) {
                    throw ShellException("No commands in multiWatch");
                }
                for (auto& command : commands) {
                    if (command != "") {
                        Pipeline* p = new Pipeline(command);  // Create a pipeline for each string inside quotes
                        p->parse();
                        pipelines.push_back(p);
                    } else {
                        cout << "Empty command in multiWatch" << endl;
                    }
                }
            } else {
                throw ShellException("Could not find '[");
            }
        } else {
            throw ShellException("No commands after multiWatch");
        }
    } else {
        return vector<Pipeline*>();  // Signifies multiWatch is not present
    }
    return pipelines;
}

// Executes the multiWatch command by creating an inotify instance and monitoring the tmp files for output
void executeMultiWatch(vector<Pipeline*>& pList, string output_file) {
    inotify_fd = inotify_init();  // Initialize inotify instance
    if (inotify_fd < 0) {
        perror("inotify_init");
        throw ShellException("Unable to create inotify instance");
    }

    vector<int> fds;  // To store file descriptos from the tmp files to read
    for (int i = 0; i < (int)pList.size(); i++) {
        pList[i]->executePipeline(true);
        string tmpFile = ".tmp." + to_string(pList[i]->pgid) + ".txt";
        int fd = open(tmpFile.c_str(), O_RDONLY);
        if (fd < 0) {
            // Handle race condition of file not being already created in the child process
            int temp_fd = open(tmpFile.c_str(), O_CREAT, 0644);
            if (temp_fd < 0) {
                perror("open");
                throw ShellException("Unable to open tmp file");
            }
            close(temp_fd);
            fd = open(tmpFile.c_str(), O_RDONLY);
        }
        fds.push_back(fd);
        int wd = inotify_add_watch(inotify_fd, tmpFile.c_str(), IN_MODIFY);  // Add watch descriptor to inotify instance
        if (wd < 0) {
            perror("inotify_add_watch");
            throw ShellException("Unable to add watch");
        }
        pgid_wd[pList[i]->pgid] = wd;  // Map process group id to watch descriptor
        wd_ind[wd] = i;                // Map watch descriptor to index in the vector
    }

    int num_running = pList.size();

    FILE* out_fp;
    if (output_file == "") {
        out_fp = stdout;
    } else {
        // Open output file for writing (if anything other than stdout)
        out_fp = fopen(output_file.c_str(), "w");
        if (out_fp == NULL) {
            perror("fopen");
            throw ShellException("Unable to open output file");
        }
    }

    int event_size = sizeof(struct inotify_event);
    int event_buf_len = 1024 * (event_size + 16);
    char event_buf[event_buf_len] __attribute__((aligned(__alignof__(struct inotify_event))));

    char buf[1024];

    // Continue watching the file descriptors until all processes are done
    while (num_running) {
        int num_read = read(inotify_fd, event_buf, sizeof(event_buf));  // Get the next event
        if (num_read < 0 && errno != EINTR && errno != EBADF) {
            perror("read");
            throw ShellException("Unable to read from inotify instance");
        } else if (errno == EBADF) {  // When inotify instance has been closed
            break;
        }

        int i = 0;
        while (i < num_read) {
            struct inotify_event* event = (struct inotify_event*)&event_buf[i];
            // IN_MODIFY implies something has been written to the file
            // IN_IGNORED implies the watch desriptor was removed from the watch list
            if ((event->mask & IN_MODIFY) || (event->mask & IN_IGNORED)) {
                int fd = fds[wd_ind[event->wd]];
                int len, seen_data = 0;
                while ((len = read(fd, buf, sizeof(buf))) > 0 || errno == EINTR) {
                    if (!seen_data) {
                        int sz = pList[wd_ind[event->wd]]->cmd.length() + to_string(time(NULL)).length() + 6;
                        string line(sz, '-');
                        fprintf(out_fp, "%s\n\"%s\", %ld :\n%s\n", line.c_str(), pList[wd_ind[event->wd]]->cmd.c_str(), time(NULL), line.c_str());
                        fflush(out_fp);
                        seen_data = 1;
                    }
                    buf[len] = '\0';
                    fprintf(out_fp, "%s", buf);  // Write to output file
                    fflush(out_fp);
                }
            }
            if (event->mask & IN_IGNORED) {
                // The watch descriptor is removed in the signal handler when the process is finished and IN_IGNORED is set
                num_running--;
                close(fds[wd_ind[event->wd]]);
            }
            i += event_size + event->len;
        }
    }

    // Delete the tmp files
    for (auto it = pgid_wd.begin(); it != pgid_wd.end(); it++) {
        string tmpFile = ".tmp." + to_string(it->first) + ".txt";
        if (remove(tmpFile.c_str()) < 0) {
            perror("remove");
            throw ShellException("Unable to remove tmp file");
        }
    }
    pgid_wd.clear();
    close(inotify_fd);
    if (out_fp != stdout) {
        fclose(out_fp);
    }
}