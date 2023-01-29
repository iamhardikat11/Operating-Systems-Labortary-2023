#include "history.h"

#include <fstream>
#include <iostream>

using namespace std;

deque<string> history;  // Stores the history in a FIFO structure
const string HIST_FILE = ".shell_history";

// Load the history contents from the file when the shell starts
void loadHistory() {
    history.clear();
    ifstream file(HIST_FILE);
    if (!file.is_open()) {
        return;
    } else {
        string line = "";
        while (getline(file, line)) {
            history.push_back(line);
        }
    }
    file.close();
}

// Returns the commands matched in the history
vector<string> searchInHistory(string s) {
    vector<string> commands;
    for (int ind = history.size() - 1; ind >= 0; ind--) {
        // Substring matching is performed using the KMP algorithm
        string hist_cmd = history[ind];
        char ch = '\0' + 229;
        string t = s + ch + hist_cmd;
        int n = t.size();
        vector<int> lps(n + 1);
        int i = 0, j = -1;
        lps[0] = -1;
        while (i < n) {
            while (j != -1 && t[j] != t[i]) {
                j = lps[j];
            }
            i++;
            j++;
            lps[i] = j;
            if (lps[i] == (int)s.size()) {
                if (s.size() == hist_cmd.size()) {  // Exact match
                    return vector<string>(1, hist_cmd);
                } else {  // Substring match
                    commands.push_back(hist_cmd);
                }
            }
        }
    }
    return (s.size() > 2 ? commands : vector<string>());
}

// Display the shell history when the 'history' command has to be executed
void printHistory() {
    int i = max(0, (int)history.size() - HIST_DISPLAY_SIZE);
    for (int cnt = 0; cnt < min((int)history.size(), HIST_DISPLAY_SIZE); i++, cnt++) {
        cout << i + 1 << " " << history[i] << endl;
    }
}

// Add a command to the history
void addToHistory(string s) {
    if (history.size() == HIST_SIZE) {
        history.pop_front();
    }
    history.push_back(s);
}

// Save the history to the file when the shell exits
void updateHistory() {
    ofstream file(HIST_FILE);
    if (!file.is_open()) {
        return;
    } else {
        for (auto it : history) {
            string temp = it + "\n";
            file << temp;
        }
    }
    file.close();
}
