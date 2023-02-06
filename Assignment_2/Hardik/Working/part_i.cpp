#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>
#include <fnmatch.h>
#include <algorithm>
using namespace std;

std::vector<std::string> list_files(const std::string &dir, const std::string &pattern)
{
    std::vector<std::string> files;
    DIR *dp;
    struct dirent *entry;
    if((dp = opendir(dir.c_str())) == NULL)
    {
        std::cerr << "Cannot open directory: " << dir << std::endl;
        return files;
    }
    while((entry = readdir(dp)) != NULL)
    {
        std::string file_name = entry->d_name;
        if(fnmatch(pattern.c_str(), file_name.c_str(), FNM_PATHNAME | FNM_PERIOD) == 0)
            files.push_back(file_name);
    }
    closedir(dp);
    return files;
}

std::vector<std::string> expand_wildcards(std::vector<std::string> args)
{
    std::vector<std::string> expanded_args;
    for(const auto &arg : args)
    {
        if(arg.find("*") == std::string::npos && arg.find("?") == std::string::npos)
        {
            expanded_args.push_back(arg);
            continue;
        }
        std::string dir = ".";
        std::string pattern = arg;
        size_t pos = arg.find_last_of("/");
        if(pos != std::string::npos)
        {
            dir = arg.substr(0, pos);
            pattern = arg.substr(pos + 1);
        }
        std::vector<std::string> files = list_files(dir, pattern);
        std::sort(files.begin(), files.end());
        for(const auto &file : files)
            expanded_args.push_back(dir + "/" + file);
    }
    return expanded_args;
}

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv + 1, argv + argc);
    std::vector<std::string> expanded_args = expand_wildcards(args);
    std::cout << "Expanded arguments: ";
    for(const auto &arg : expanded_args)
        std::cout << arg << " ";
    cout << endl;
    for(int i=1;i<expanded_args.size();i++)
    {
        cout << expanded_args[0] << " " << expanded_args[i] << endl;
    }
    std::cout << std::endl;
    return 0;
}
