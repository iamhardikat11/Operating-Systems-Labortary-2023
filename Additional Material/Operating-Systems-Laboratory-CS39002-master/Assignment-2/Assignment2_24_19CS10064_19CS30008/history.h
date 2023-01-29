#ifndef __HISTORY_H
#define __HISTORY_H

#include <deque>
#include <string>
#include <vector>

using namespace std;

#define HIST_SIZE 10000
#define HIST_DISPLAY_SIZE 1000

extern deque<string> history;

void loadHistory();
vector<string> searchInHistory(string s);
void printHistory();
void addToHistory(string s);
void updateHistory();

#endif