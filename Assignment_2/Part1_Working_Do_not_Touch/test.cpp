#include <iostream>
#include <vector>
#include <dirent.h>
#include <string.h>

using namespace std;

bool match(const char *pat, const char *str)
{
    int i, j;
    for (i = 0, j = 0; pat[j]; i++, j++)
    {
        if (pat[j] == '*')
        {
            int k = j++;
            while (pat[j] == '*')
                j++;
            if (!pat[j])
                return true;
            while (str[i])
            {
                if (match(pat + j, str + i))
                    return true;
                i++;
            }
            return false;
        }
        else if (pat[j] != '?' && pat[j] != str[i])
            return false;
    }
    return !str[i];
}

vector<string> expand(const string &s)
{
    vector<string> result;
    string path, pattern;
    size_t pos = s.rfind('/');
    if (pos == string::npos)
    {
        path = ".";
        pattern = s;
    }
    else
    {
        path = s.substr(0, pos);
        pattern = s.substr(pos + 1);
    }
    DIR *dir = opendir(path.c_str());
    if (!dir)
    {
        result.push_back(s);
        return result;
    }
    dirent *ent;
    while ((ent = readdir(dir)))
    {
        if (match(pattern.c_str(), ent->d_name))
        {
            string file = path + "/" + ent->d_name;
            result.push_back(file);
        }
    }
    closedir(dir);
    return result;
}

int main(int argc, char *argv[])
{
    vector<string> args;
    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];
        int len = strlen(arg);
        printf("%s\n", arg);
        bool wild = false;
        for (int j = 0; j < len; j++)
        {
            if (arg[j] == '*' || arg[j] == '?')
            {
                wild = true;
                break;
            }
        }
        if (wild)
        {
            vector<string> files = expand(arg);
            args.insert(args.end(), files.begin(), files.end());
        }
        else
        {
            args.push_back(arg);
        }
    }
    cout << "Expanded arguments:" << endl;
    for (string &arg : args)
    {
        cout << arg << endl;
    }
    return 0;
}
