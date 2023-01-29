#ifndef __AUTOCOMPLETE_H
#define __AUTOCOMPLETE_H

#include <string>
#include <vector>

using namespace std;

vector<string> getFilesInCurrDir();
vector<string> autocomplete(string s);

#endif