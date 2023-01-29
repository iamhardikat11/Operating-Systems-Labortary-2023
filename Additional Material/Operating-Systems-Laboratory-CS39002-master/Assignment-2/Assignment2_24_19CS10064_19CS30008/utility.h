#ifndef __UTILITY_H
#define __UTILITY_H

#include <string>
#include <vector>

using namespace std;

void trim(string& s);
vector<string> split(string& str, char delim);
vector<char*> cstrArray(vector<string>& args);

#endif