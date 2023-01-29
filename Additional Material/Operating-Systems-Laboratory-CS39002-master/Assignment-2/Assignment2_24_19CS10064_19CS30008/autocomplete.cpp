#include "autocomplete.h"

#include <dirent.h>
#include <sys/types.h>

#include <algorithm>
#include <iostream>

using namespace std;

// Returns a vector of strings containing the names of all files in the current directory
vector<string> getFilesInCurrDir() {
    vector<string> files;
    DIR* dir = opendir(".");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {    // Navigate the directory using readdir
            DIR* sub_dir = opendir(entry->d_name);  // If the entry cannot be opened, it means it is a file
            if (sub_dir) {
                closedir(sub_dir);
            } else {
                string file(entry->d_name);
                files.push_back(file);
            }
        }
        closedir(dir);
    } else {
        cout << "Directory could not be opened";
    }
    return files;
}

// Returns the autocomplete suggestions for a query string
vector<string> autocomplete(string s) {
    vector<string> filenames = getFilesInCurrDir();
    vector<string> matched;
    int s_len = s.length();
    for (int i = 0; i < (int)filenames.size(); i++) {
        if ((int)filenames[i].length() < s_len) {
            continue;
        } else if (s == filenames[i].substr(0, s_len)) {  // Check if s is a prefix of filenames[i]
            matched.push_back(filenames[i]);
        }
    }

    if ((int)matched.size() <= 1) {  // Single match
        return matched;
    } else {
        // Try to find a common prefix in all the suggestions matched
        sort(matched.begin(), matched.end());
        int i = 0;
        string suggest = "";
        while (i < (int)min(matched.front().size(), matched.back().size()) && matched.front()[i] == matched.back()[i]) {
            suggest += matched[0][i++];
        }
        if (suggest.length() > s.length()) {
            return vector<string>(1, suggest);
        } else {
            return matched;
        }
    }
}
