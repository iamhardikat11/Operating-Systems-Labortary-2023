#include "utility.h"

#include <cstring>
#include <sstream>

using namespace std;

// Removes whitespaces from beginning and end of a string
void trim(string& s) {
    while (s.length() && s.back() == ' ') {
        s.pop_back();
    }
    int i = 0;
    while (i < (int)s.length() && s[i] == ' ') {
        i++;
    }
    s = s.substr(i);
}

// Splits an input string on the basis of a delimiter
vector<string> split(string& str, char delim) {
    vector<string> tokens;
    stringstream ss(str);
    string tmp;
    while (getline(ss, tmp, delim)) {
        tokens.push_back(tmp);
    }
    return tokens;
}

// Converts a vector of strings to a vector of char*
vector<char*> cstrArray(vector<string>& args) {
    vector<char*> args_(args.size() + 1);
    for (int i = 0; i < (int)args.size(); i++) {
        args_[i] = (char*)malloc((args[i].length() + 1) * sizeof(char));
        strcpy(args_[i], args[i].c_str());
    }
    args_[args.size()] = (char*)malloc(sizeof(char));
    args_[args.size()] = nullptr;
    return args_;
}
