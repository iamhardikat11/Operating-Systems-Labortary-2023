#include <iostream>
#include <string>
#include <vector>
#include <glob.h>
#include <cstring>
#include <sstream>
using namespace std;
void expandWildcards(const std::string &arg, std::vector<std::string> &args)
{
    glob_t globBuffer;
    memset(&globBuffer, 0, sizeof(globBuffer));
    int globResult = glob(arg.c_str(), GLOB_TILDE, NULL, &globBuffer);
    if (globResult == 0)
    {
        for (size_t i = 0; i < globBuffer.gl_pathc; ++i)
        {
            args.push_back(globBuffer.gl_pathv[i]);
        }
    }
    globfree(&globBuffer);
}

int main()
{
    std::vector<std::string> args;
    string command;
    std::cout << "Enter a Regex:- ";
    getline(cin,command);
    std::string arg;
    std::stringstream ss(command);
    string c = "";
    for(int i=0;command[i]!=' '; i++)
        c.push_back(command[i]);
    while (ss >> arg)
        expandWildcards(arg, args);
    std::cout << "Expanded arguments:" << std::endl;
    std::vector<std::string> args_standard;
    for (const auto &arg : args)
        args_standard.push_back(c + " " + arg);
    return 0;
}
